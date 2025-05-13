#include "GWBManager.h"
#include "GWBRuntimeModule.h"
#include "CVars.h"
#include "GWBSubsystem.h"
#include "Stats.h"
#include "DataTypes/GWBTimeSlicedScope.h"
#include "DataTypes/GWBTimeSliceResetScope.h"
#include "DataTypes/GWBWorkUnitHandle.h"
#include "Utils/TSetUtils.h"

void UGWBManager::Initialize()
{
	Scheduler = NewObject<UGWBScheduler>();
	Budgeter = NewObject<UGWBBudgeter>();
	
	// Generate work categories from definitions
	for (auto& Def : WorkGroupDefinitions)
	{
		WorkGroups.Add(FGWBWorkGroup(Def));
	}
	
	// Scheduler->StartWorkCycleDelegate.BindRaw(this, &UGWBManager::DoWork);
}
void UGWBManager::Reset()
{
	if (bIsDoingWork)
	{
		bPendingReset = true;
		return;
	}
	OnBeforeDoWorkDelegate.Clear();
	bIsDoingWork = false;
	bPendingReset = false;
	// EscalationScalar = 0.0;
	Scheduler->Stop();
	Budgeter->Reset();
	for (auto ItCategory = WorkGroups.CreateIterator(); ItCategory; ++ItCategory)
	{
		for (auto& WorkUnit : ItCategory->WorkUnitsQueue)
		{
			WorkUnit.GetAbortCallback().ExecuteIfBound();
			WorkUnit.MarkAborted();
		}
	}
	WorkGroups.Reset();
}
FGWBWorkUnitHandle UGWBManager::ScheduleWork(const UObject* WorldContextObject, const FName WorkGroupId, const FGWBWorkOptions& WorkOptions)
{
	const UGWBSubsystem* Subsystem = GEngine->GetEngineSubsystem<UGWBSubsystem>();
	UGWBManager* GlobalManager = Subsystem->GetManager();
	return GlobalManager->ScheduleWork(WorkGroupId, WorkOptions);	
}
void UGWBManager::AbortWorkUnit(const UObject* WorldContextObject, FGWBWorkUnitHandle WorkUnit)
{
	WorkUnit.GetAbortCallback().ExecuteIfBound();
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
	if (!ensureAlwaysMsgf(WorkGroupIndex.IsValidId(), TEXT("ScheduleWorkUnit -> Invalid WorkGroupId: %s"), *WorkGroupId.ToString())) return FGWBWorkUnitHandle();
	auto& WorkGroup = WorkGroups[WorkGroupIndex];
	
	// schedule a unit of work with the provided options and callback
	const double CurrentTime = FPlatformTime::Seconds();
	FGWBWorkUnit WorkUnit(WorkOptions, CurrentTime);

	// Figure out the priority index of the work unit
	const int32 InsertIndex = Algo::LowerBoundBy(WorkGroup.WorkUnitsQueue, WorkUnit,
	                                             [](const FGWBWorkUnit& ExistingWorkUnit) { return ExistingWorkUnit; },
	                                             [](const FGWBWorkUnit& ExistingWorkUnit, const FGWBWorkUnit& WorkUnit)
	                                             {
		                                             return ExistingWorkUnit.Options.Priority <= WorkUnit.Options.Priority;
	                                             });

	// Insert sort the unit of work instance into the group's work unit
	const auto NewIndex = WorkGroup.WorkUnitsQueue.Insert(WorkUnit, InsertIndex);
	
	Budgeter->OnWorkUnitScheduled();
	TotalWorkCount++;

	UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGameplayWorkBalancer::ScheduleWorkUnit -> Group: %s, Instance %d (GroupWorkCount: %d, GlobalWorkCount: %d)"), *WorkGroupId.ToString(), WorkUnit.GetId(), WorkGroup.WorkUnitsQueue.Num(), Budgeter->WorkUnitsCount);

	Scheduler->Start();

	return FGWBWorkUnitHandle(WorkUnit);
};
void UGWBManager::DoWork()
{
	SCOPE_CYCLE_COUNTER(STAT_DoWorkForFrame);

	const double TimeSinceLastWork = FPlatformTime::Seconds() - Budgeter->LastWorkTimestamp;
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
	
	Budgeter->Start();

	// Do work for each group
	{
		SCOPE_CYCLE_COUNTER(STAT_DoWorkForFrame_Groups);

		// when this struct goes out of scope it's destructor will reset the time slicer we use to budget the gameplay work balancer
		FGWBTimeSliceResetScope ResetTimeSlicer(this, FName("GameplayWorkBalancer"));

		UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWork -> Start (NumGroups: %d, GlobalWorkCount: %d, RemainingTimeBudget: %f)"),
			WorkGroupsWithWork.Num(),
			Budgeter->WorkUnitsCount,
			Budgeter->GetRemainingTimeInBudget(),
		);
		
		for (auto& WorkGroup : WorkGroups)
		{
			// this scoped struct will increment the time slicer within this for loop
			FGWBTimeSlicedScope TimeSlicedWork(this, FName("GameplayWorkBalancer"), (double)CVarGWB_FrameBudget.GetValueOnGameThread(), 0);
			
			// if there's no work to be done, skip this group
			if (WorkGroup.WorkUnitsQueue.Num() == 0) continue;
			
			// 
			Budgeter->StartGroup(WorkGroup.Def.MaxFrameBudget, WorkGroup.Def.MaxWorkUnitsPerFrame);

			if (TimeSlicedWork.IsOverBudget())
			{
				UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWork -> OVER BUDGET. Skip Group: %s (NumSkippedFrames: %d MaxNumSkippedFrames: %d)"), *WorkGroup.Def.Id.ToString(), WorkGroup.NumSkippedFrames, WorkGroup.Def.MaxNumSkippedFrames);
			}
			
			// we are out of time based on our budget
			const auto RemainingBudget = Budgeter->BudgetForFrameExceededTime - FPlatformTime::Seconds();
			if (RemainingBudget <= 0)
			{
				UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWork -> Skip Group: %s, Num Skipped: %d (max: %d), Reason: Over frame budget"), *WorkGroup.Def.Id.ToString(), WorkGroup.NumSkippedFrames, WorkGroup.Def.MaxNumSkippedFrames);
				WorkGroup.NumSkippedFrames++;
				// if we skipped work for this group, use it's configuration to control how much to escalate it's own priority when skipped
				WorkGroup.PriorityOffset += WorkGroup.Def.SkipPriorityDelta;
				break;
			}

			// Do work for group and record when work was completed
			WorkGroup.NumSkippedFrames = 0;
			WorkGroup.PriorityOffset = 0;
			DoWorkForGroup(WorkGroup);
		}

		UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWork -> End (NumGroups: %d, WorkUnitsDoneThisCycle: %d, TimeSpent: %.3f)"),
			WorkGroupsWithWork.Num(),
			Budgeter->WorkUnitsDoneThisCycle,
			FPlatformTime::Seconds() - Budgeter->LastWorkTimestamp
		);
	}

	// Handle work group priority changes
	{
		SCOPE_CYCLE_COUNTER(STAT_DoWorkForFrame_Reprioritize);
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

	// const bool NeedsToScheduleNextFrame = TSetUtils::Reduce<FGWBWorkGroup, bool>(WorkGroups, [](bool Accumulator, const FGWBWorkGroup& WorkGroup)
	// {
	// 	return Accumulator || WorkGroup.WorkUnitsQueue.Num() > 0;
	// }, false);

	// // figure out if there is any work remaining in any of the work groups
	// const bool NeedsToScheduleNextFrame = [&](){
	// 	for (auto& WorkGroup : WorkGroups)
	// 	{
	// 		if (WorkGroup.WorkUnitsQueue.Num() > 0)
	// 		{
	// 			return true;
	// 		}
	// 	}
	// 	return false;
	// }();

	// if we have still have work, schedule the next frame
	const bool NeedsToScheduleNextFrame = Budgeter->WorkUnitsCount > 0;
	if (NeedsToScheduleNextFrame)
	{
		Scheduler->Start();
	}
};
void UGWBManager::DoWorkForGroup(FGWBWorkGroup& WorkGroup)
{
	SCOPE_CYCLE_COUNTER(STAT_DoWorkForGroup);

	// when this struct goes out of scope it's destructor will reset the time slicer we use to budget the group
	FGWBTimeSliceResetScope ResetTimeSlicerForGroup(this, WorkGroup.Def.Id);

	UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWorkForGroup -> Group: %s, Instances: %d, Start: %.3f, Max: %.3f"),
		*WorkGroup.Def.Id.ToString(),
		WorkGroup.WorkUnitsQueue.Num(),
		FPlatformTime::Seconds(),
		Budgeter->BudgetForGroupExceededTimestamp);

	for (int32 i = 0; i < WorkGroup.WorkUnitsQueue.Num(); i++)
	{
		// this scoped struct will increment the time slicer within this for loop
		FGWBTimeSlicedScope TimeSlicedWork(this, WorkGroup.Def.Id, WorkGroup.Def.MaxFrameBudget, WorkGroup.Def.MaxWorkUnitsPerFrame);

		if (TimeSlicedWork.IsOverUnitCountBudget())
		{
			UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWorkForGroup -> OVER GROUP UNIT COUNT BUDGET Group: %s, Units Remaining: %d"),
				*WorkGroup.Def.Id.ToString(),
				WorkGroup.WorkUnitsQueue.Num());
		}

		if (TimeSlicedWork.IsOverFrameTimeBudget())
		{
			UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWorkForGroup -> OVER GROUP TIME BUDGET Group: %s, Units Remaining: %d"),
				*WorkGroup.Def.Id.ToString(),
				WorkGroup.WorkUnitsQueue.Num());
		}
		
		auto& WorkUnit = WorkGroup.WorkUnitsQueue[i];

		// START budget checks
		if (Budgeter->HasCurrentGroupUnitBudgetBeenExceeded())
		{
			UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWorkForGroup -> Reached max %d InstancesPerFrame for Group: %s, Instance %d (remaining: %d, global: %d)"),
					WorkGroup.Def.MaxWorkUnitsPerFrame,
					*WorkGroup.Def.Id.ToString(),
					i,
					WorkGroup.WorkUnitsQueue.Num(),
					Budgeter->WorkUnitsCount);
			break; // BREAK if we've reached MAX count of units of work in this group allowed
		}
		const double StartWorkTimestamp = FPlatformTime::Seconds();
		const double TimeElapsedSinceScheduled = StartWorkTimestamp - WorkUnit.ScheduledTimestamp;
		const bool HasUnitExceedMaxIdleTime = WorkUnit.Options.MaxDelay > 0 && TimeElapsedSinceScheduled > WorkUnit.Options.MaxDelay;
		// TODO: Don't use max delay directly, as that could cause all instances scheduled at the same time to also do work at the same time
		if (!HasUnitExceedMaxIdleTime && Budgeter->HasCurrentGroupTimeBudgetBeenExceeded())
		{
			UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWorkForGroup -> Exceeded time for Group: %s, Instance %d (remaining: %d, global: %d), Start: %.3f"),
				*WorkGroup.Def.Id.ToString(),
				i,
				WorkGroup.WorkUnitsQueue.Num(),
				Budgeter->WorkUnitsCount,
				StartWorkTimestamp);
			break; // BREAK if we've run out of time budget for this group
		}
		// END budget checks
		
		// Skip instance if aborted
		if (WorkUnit.HasWork())
		{
			Budgeter->OnWorkUnitStarted();
			DoWorkForUnit(WorkUnit);
			const double EndWorkTimestamp = FPlatformTime::Seconds();
			const double UnitWorkDeltaTime = EndWorkTimestamp - StartWorkTimestamp;
			Budgeter->OnWorkUnitComplete();
			Budgeter->RecordGroupTelemetry(WorkGroup.Def.Id, UnitWorkDeltaTime);

			TotalWorkCount--;
			
			UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBManager::DoWorkForGroup -> Did work for Group: %s, Instance %d (remaining: %d, global: %d), Start: %.3f, End: %.3f, Delta: %.3f, Avg: %.3f"),
				*WorkGroup.Def.Id.ToString(),
				WorkUnit.GetId(),
				WorkGroup.WorkUnitsQueue.Num() - (WorkUnit.HasWork() ? 0 : 1),
				Budgeter->WorkUnitsCount - (WorkUnit.HasWork() ? 0 : 1),
				StartWorkTimestamp,
				EndWorkTimestamp,
				EndWorkTimestamp - StartWorkTimestamp,
				WorkGroup.AverageUnitTime
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
	// technically it would be better if this was passed in from outside for consistency
	// since time has elapsed to do a few instructions since, but API is much simpler when just estimating it here
	const double MaxGroupTimestamp = Budgeter->BudgetForGroupExceededTimestamp;
	const double StartInstanceTime = FPlatformTime::Seconds();
	const float Budget = static_cast<float>(MaxGroupTimestamp - StartInstanceTime);
	const float TimeSinceScheduled = static_cast<float>(StartInstanceTime - WorkUnit.ScheduledTimestamp);

	// do the work!
	WorkUnit.GetWorkCallback().ExecuteIfBound(TimeSinceScheduled, FGWBWorkUnitHandle());
	WorkUnit.MarkCompleted();
};