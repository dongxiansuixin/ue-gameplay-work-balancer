#include "GWBManager.h"
#include "GWBRuntimeModule.h"
#include "GWBSubsystem.h"
#include "DataTypes/GWBTimeSlicedLoopScope.h"
#include "DataTypes/GWBTimeSliceScopedHandle.h"
#include "DataTypes/GWBWorkUnitHandle.h"
#include "Extensions/Modifiers.h"
#include "Stats.h"
#include "CVars.h"

#define TO_MS_STRING(MS) *FString::Printf(TEXT("%.2fms"), (MS*1000))

UGWBManager::UGWBManager()
{
	FGWBWorkGroupDefinition Default;
	Default.Id = FName(TEXT("Default"));
	WorkGroupDefinitions.Add(Default);
}

void UGWBManager::Initialize(UWorld* ForWorld)
{
	UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::Initialize -> Group Count: %d"), WorkGroupDefinitions.Num());
	
	Scheduler = NewObject<UGWBScheduler>(ForWorld);
	
	// Generate work categories from definitions
	for (auto& Def : WorkGroupDefinitions)
	{
		WorkGroups.Add(FGWBWorkGroup(Def));
	}

	ModifierManager.AddBudgetModifier(FFrameBudgetEscalationModifier());

	Scheduler->StartWorkCycleDelegate.BindLambda([&]()
	{
		DoWork();
	});
}

TArray<FName> UGWBManager::GetValidGroupNames() const
{
	TArray<FName> Names;
	Names.Reserve(WorkGroupDefinitions.Num());
	for (auto Group : WorkGroupDefinitions)
	{
		Names.Add(Group.Id);
	}
	return Names;
}

void UGWBManager::Reset()
{
	if (bIsDoingWork)
	{
		UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::Reset -> scheduled reset because there is still work"));
		bPendingReset = true;
		return;
	}
	UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::Reset -> TotalWorkCount: %d"), TotalWorkCount);
	OnBeforeDoWorkDelegate.Clear();
	bIsDoingWork = false;
	bPendingReset = false;
	if (Scheduler != nullptr) Scheduler->Stop();
	for (auto ItCategory = WorkGroups.CreateIterator(); ItCategory; ++ItCategory)
	{
		for (auto& WorkUnit : ItCategory->WorkUnitsQueue)
		{
			WorkUnit.GetAbortCallback().ExecuteIfBound();
			WorkUnit.MarkAborted();
		}
	}
	TotalWorkCount = 0;
	WorkGroups.Reset();
}
FGWBWorkUnitHandle UGWBManager::ScheduleWork(const UObject* WorldContextObject, const FName WorkGroupId, const FGWBWorkOptions& WorkOptions)
{
	const UGWBSubsystem* Subsystem = GEngine->GetEngineSubsystem<UGWBSubsystem>();
	UGWBManager* GlobalManager = Subsystem->GetManager();
	return GlobalManager->ScheduleWork(WorkGroupId, WorkOptions);	
}
void UGWBManager::AbortWorkUnit(const UObject* WorldContextObject, FGWBWorkUnitHandle WorkUnitHandle)
{
	const UGWBSubsystem* Subsystem = GEngine->GetEngineSubsystem<UGWBSubsystem>();
	UGWBManager* GlobalManager = Subsystem->GetManager();
	for (auto ItCategory = GlobalManager->WorkGroups.CreateIterator(); ItCategory; ++ItCategory)
	{
		for (auto& WorkUnit : ItCategory->WorkUnitsQueue)
		{
			if (WorkUnitHandle.GetId() == WorkUnit.GetId())
			{
				WorkUnit.GetAbortCallback().ExecuteIfBound();
				WorkUnit.MarkAborted();
				return;
			}
		}
	}
}

void UGWBManager::BindBlueprintCallback(FGWBWorkUnitHandle& Handle, const FGWBBlueprintWorkDelegate& OnDoWork)
{
	Handle.OnHandleWork([OnDoWork](float TimeSinceScheduled, const FGWBWorkUnitHandle& Handle) {
		OnDoWork.ExecuteIfBound(TimeSinceScheduled, Handle);
	});
}

FGWBWorkUnitHandle UGWBManager::ScheduleWork(const FName& WorkGroupId, const FGWBWorkOptions& WorkOptions)
{
	// if the game balancer is disabled, just do the work
	if (!CVarGWB_Enabled.GetValueOnGameThread())
	{
		return FGWBWorkUnitHandle::PassthroughHandle();
	}
	
	SCOPE_CYCLE_COUNTER(STAT_ScheduleWorkUnit);

	// Grab the work group data for the group ID we are scheduling the unit of work for 
	const FSetElementId WorkGroupIndex = WorkGroups.FindId(WorkGroupId);
	if (!ensureAlwaysMsgf(WorkGroupIndex.IsValidId(), TEXT("ScheduleWorkUnit -> Invalid WorkGroupId: %s"), *WorkGroupId.ToString())) return FGWBWorkUnitHandle::PassthroughHandle();
	auto& WorkGroup = WorkGroups[WorkGroupIndex];
	
	// schedule a unit of work with the provided options and callback
	const double CurrentTime = FPlatformTime::Seconds();
	FGWBWorkUnit WorkUnit(WorkOptions, CurrentTime);

	// Figure out the priority index of the work unit
	const int32 InsertIndex = Algo::LowerBoundBy(WorkGroup.WorkUnitsQueue, WorkUnit,
		[](const FGWBWorkUnit& ExistingWorkUnit) { return ExistingWorkUnit; },
		[](const FGWBWorkUnit& ExistingWorkUnit, const FGWBWorkUnit& WorkUnit)
		{
			return ExistingWorkUnit.GetEffectivePriority() <= WorkUnit.GetEffectivePriority();
		});

	// Insert sort the unit of work instance into the group's work unit
	WorkGroup.WorkUnitsQueue.Insert(WorkUnit, InsertIndex);
	
	TotalWorkCount++;
	SET_DWORD_STAT(STAT_GameWorkBalancer_WorkCount, TotalWorkCount);
	
	Scheduler->Start();

	UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::ScheduleWork\t-> Group: %s, Instance %d\t\t(GroupWorkCount: %d, GlobalWorkCount: %d)"),
			*WorkGroupId.ToString(),
			WorkUnit.GetId(),
			WorkGroup.WorkUnitsQueue.Num(),
			TotalWorkCount);

	// allow extensions to react to work scheduling
	OnWorkScheduled(WorkGroupId);

	return FGWBWorkUnitHandle(WorkUnit);
};
void UGWBManager::DoWork()
{
	SCOPE_CYCLE_COUNTER(STAT_DoWorkForFrame);

	// allow extensions to plug in to modify the frame budget
	double FrameBudget = (double)CVarGWB_FrameBudget.GetValueOnGameThread();
	ApplyBudgetModifiers(FrameBudget);
	int WorkCountBudget = CVarGWB_WorkCountBudget.GetValueOnGameThread();

	// when this struct goes out of scope it's destructor will reset the time slicer we use to budget the gameplay work balancer
	FGWBTimeSliceScopedHandle TimeSlicer(this, FName("GameplayWorkBalancer"), FrameBudget, WorkCountBudget);

	const double TimeSinceLastWork = FPlatformTime::Seconds() - TimeSlicer.GetLastResetTimestamp();
	OnBeforeDoWorkDelegate.Broadcast(TimeSinceLastWork);
	bIsDoingWork = true;

	// TODO: this can be optimized as in original implementation by doing the counts in the Scheduling Functions
	TSet<FName> WorkGroupsWithWork;
	for (auto Group : WorkGroups)
	{
		if (Group.WorkUnitsQueue.Num() > 0)
		{
			WorkGroupsWithWork.Add(Group.Def.Id);
		}
	}

	// Do work for each group
	{
		SCOPE_CYCLE_COUNTER(STAT_DoWorkForFrame_Groups);

		UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWork\t-> START\t(NumGroups: %d, GlobalWorkCount: %d, TimeBudget: %s)"),
			WorkGroupsWithWork.Num(),
			TotalWorkCount,
			TO_MS_STRING(FrameBudget)
		);

		uint32 i = 0;
		for (auto& WorkGroup : WorkGroups)
		{
			// if there's no work to be done, skip this group
			if (WorkGroup.WorkUnitsQueue.Num() == 0) continue;

			if (TimeSlicer.IsOverBudget())
			{
				UE_LOG(Log_GameplayWorkBalancer, Log, TEXT("UGWBManager::DoWork\t-> OVER BUDGET\t - Skip Group: %s (NumSkippedFrames: %d MaxNumSkippedFrames: %d)"), *WorkGroup.Def.Id.ToString(), WorkGroup.NumSkippedFrames, WorkGroup.Def.MaxNumSkippedFrames);
				WorkGroup.NumSkippedFrames++;
				// if we skipped work for this group, use it's configuration to control how much to escalate it's own priority when skipped
				WorkGroup.PriorityOffset += WorkGroup.Def.SkipPriorityDelta;

				// Track deferred work groups
				for (int32 j = i; j < WorkGroup.WorkUnitsQueue.Num(); j++)
				{
					OnWorkGroupDeferred(WorkGroup.Def.Id);
				}
				
				break;
			}

			// Do work for group and record when work was completed
			WorkGroup.NumSkippedFrames = 0;
			WorkGroup.PriorityOffset = 0;
			DoWorkForGroup(WorkGroup);

			i++;
		}

		double TimeSpent = FPlatformTime::Seconds() - TimeSlicer.GetLastResetTimestamp();
		UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWork\t-> END\t\t(NumGroups: %d, WorkUnitsDoneThisCycle: %d, TimeSpent: %s)"),
			WorkGroupsWithWork.Num(),
			TimeSlicer.GetWorkUnitsCompleted(),
			TO_MS_STRING(TimeSpent)
		);
	}

	// Handle work group priority changes
	{
		SCOPE_CYCLE_COUNTER(STAT_DoWorkForFrame_Reprioritize);
		
		// // Apply priority modifiers
		// for (auto& WorkGroup : WorkGroups)
		// {
		// 	int32 Priority = WorkGroup.GetPriority();
		// 	for (const auto& Modifier : PriorityModifiers)
		// 	{
		// 		if (Modifier.GetInterface())
		// 		{
		// 			Modifier->ModifyGroupPriority(WorkGroup.Def.Id, Priority);
		// 		}
		// 	}
		// 	// Update priority offset based on modifiers
		// 	int32 BaseOffset = Priority - WorkGroup.Def.Priority;
		// 	// Only update if there was a change from modifiers (preserve the SkipPriorityDelta changes)
		// 	if (BaseOffset != 0)
		// 	{
		// 		WorkGroup.PriorityOffset = BaseOffset;
		// 	}
		// }
		
		// Sort work groups by modified priority
		WorkGroups.Sort([&](const FGWBWorkGroup& A, const FGWBWorkGroup& B){
			return A.GetPriority() < B.GetPriority();
		});
	}

	bIsDoingWork = false;

	if (bPendingReset)
	{
		Reset();
		return;
	}

	// if we have still have work, schedule the next frame
	const bool NeedsToScheduleNextFrame = TotalWorkCount > 0;
	if (NeedsToScheduleNextFrame)
	{
		ModifierManager.NotifyWorkDeferred(TotalWorkCount);
		Scheduler->Start();
	}
};
void UGWBManager::DoWorkForGroup(FGWBWorkGroup& WorkGroup)
{
	SCOPE_CYCLE_COUNTER(STAT_DoWorkForGroup);

	// when this struct goes out of scope it's destructor will reset the time slicer we use to budget the group
	FGWBTimeSliceScopedHandle GroupTimeSliceHandle(this, WorkGroup.Def.Id);
	
	// allow extensions to plug in to modify the frame budget
	double FrameBudget = (double)CVarGWB_FrameBudget.GetValueOnGameThread();
	ApplyBudgetModifiers(FrameBudget);
	int WorkCountBudget = CVarGWB_WorkCountBudget.GetValueOnGameThread();

	// Apply group-specific budget modifiers
	double GroupTimeBudget = WorkGroup.Def.MaxFrameBudget;
	int32 GroupUnitCount = WorkGroup.Def.MaxWorkUnitsPerFrame;
	ApplyGroupBudgetModifiers(WorkGroup.Def.Id, GroupTimeBudget, GroupUnitCount);

	for (int32 i = 0; i < WorkGroup.WorkUnitsQueue.Num(); i++)
	{
		// this scoped struct will increment the time slicer within this for loop
		FGWBTimeSlicedLoopScope TimeSlicedGroupWork(this, WorkGroup.Def.Id, GroupTimeBudget, GroupUnitCount); // budget for group
		FGWBTimeSlicedLoopScope TimeSlicedWork(this, FName("GameplayWorkBalancer"), FrameBudget, WorkCountBudget); // budget for all work

		auto& WorkUnit = WorkGroup.WorkUnitsQueue[i];

		// START budget checks
		// BREAK if we've reached MAX count of units of work in this group allowed
		if (TimeSlicedGroupWork.IsOverUnitCountBudget() ||
			TimeSlicedWork.IsOverUnitCountBudget())
		{
			UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWorkForGroup \"%s\"\t -> OVER GROUP UNIT COUNT BUDGET, WorkUnits Remaining: %d"),
				*WorkGroup.Def.Id.ToString(),
				WorkGroup.WorkUnitsQueue.Num());
				
			// Track deferred work units
			for (int32 j = i; j < WorkGroup.WorkUnitsQueue.Num(); j++)
			{
				OnWorkUnitDeferred(WorkGroup.Def.Id);
			}
			
			break;
		}

		// BREAK if we've run out of time budget for this group
		if (TimeSlicedGroupWork.IsOverFrameTimeBudget() ||
			TimeSlicedWork.IsOverFrameTimeBudget())
		{
			UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWorkForGroup \"%s\"\t -> OVER GROUP TIME BUDGET, WorkUnits Remaining: %d"),
				*WorkGroup.Def.Id.ToString(),
				WorkGroup.WorkUnitsQueue.Num());
			
			const double StartWorkTimestamp = FPlatformTime::Seconds();
			const double TimeElapsedSinceScheduled = StartWorkTimestamp - WorkUnit.ScheduledTimestamp;
			const bool HasUnitExceedMaxIdleTime = WorkUnit.Options.MaxDelay > 0 && TimeElapsedSinceScheduled > WorkUnit.Options.MaxDelay;
			// TODO: Don't use max delay directly, as that could cause all instances scheduled at the same time to also do work at the same time
			if (!HasUnitExceedMaxIdleTime)
			{
				// Track deferred work units
				for (int32 j = i; j < WorkGroup.WorkUnitsQueue.Num(); j++)
				{
					OnWorkUnitDeferred(WorkGroup.Def.Id);
				}
				
				break; // BREAK if we've run out of time budget for this group
			}
		}
		// END budget checks
		
		// Skip instance if aborted
		if (WorkUnit.HasWork())
		{
			const double StartWorkTimestamp = FPlatformTime::Seconds();
			DoWorkForUnit(WorkUnit);
			const double EndWorkTimestamp = FPlatformTime::Seconds();
			const double UnitWorkDeltaTime = EndWorkTimestamp - StartWorkTimestamp;

			TotalWorkCount--;
			SET_DWORD_STAT(STAT_GameWorkBalancer_WorkCount, TotalWorkCount);
	
			ModifierManager.NotifyWorkComplete(TotalWorkCount);
			
			UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWorkForGroup \"%s\"\t -> Completed Instance %d\t(remaining: %d, global: %d), Start: %.3f, End: %.3f, Delta: %s, Avg: %s, RemainingTimeInBudget: %s"),
				*WorkGroup.Def.Id.ToString(),
				WorkUnit.GetId(),
				WorkGroup.WorkUnitsQueue.Num() - (WorkUnit.HasWork() ? 0 : 1),
				TotalWorkCount,
				StartWorkTimestamp,
				EndWorkTimestamp,
				TO_MS_STRING(UnitWorkDeltaTime),
				TO_MS_STRING(WorkGroup.AverageUnitTime),
				TO_MS_STRING(TimeSlicedWork.GetRemainingTimeInBudget())
			);
		}

		if (!WorkUnit.HasWork())
		{
			if (WorkUnit.Options.MaxDelay > 0.f)
			{
				WorkGroup.NumWorkUnitsWithMaxDelay--;
			}
			WorkGroup.WorkUnitsQueue.RemoveAt(i, 1, EAllowShrinking::No);
			i--;
		}
	}
};
void UGWBManager::DoWorkForUnit(const FGWBWorkUnit& WorkUnit) const
{
	SCOPE_CYCLE_COUNTER(STAT_DoWorkForUnit);
	const double StartInstanceTime = FPlatformTime::Seconds();
	const float TimeSinceScheduled = static_cast<float>(StartInstanceTime - WorkUnit.ScheduledTimestamp);

	// do the work!
	WorkUnit.GetWorkCallback().ExecuteIfBound(TimeSinceScheduled, FGWBWorkUnitHandle(WorkUnit));
	WorkUnit.MarkCompleted();
};


void UGWBManager::ApplyBudgetModifiers(double& FrameBudget)
{
	ModifierManager.ProcessBudgetModifiers(FrameBudget);
}

void UGWBManager::ApplyGroupBudgetModifiers(FName GroupId, double& TimeBudget, int32& UnitCountBudget)
{
	// TODO: extensions could use this
}

void UGWBManager::OnWorkScheduled(FName GroupId)
{
	ModifierManager.NotifyWorkScheduled(TotalWorkCount);
}

void UGWBManager::OnWorkDeferred(uint32 DeferredWorkUnitCount)
{
	ModifierManager.NotifyWorkDeferred(DeferredWorkUnitCount);
}

void UGWBManager::OnWorkGroupDeferred(FName WorkGroupId)
{
	// TODO: extensions could use this
}

void UGWBManager::OnWorkUnitDeferred(FName WorkGroupId)
{
	// TODO: extensions could use this
};