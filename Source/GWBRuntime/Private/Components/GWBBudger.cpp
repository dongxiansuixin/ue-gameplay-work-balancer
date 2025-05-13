#include "Components/GWBBudgeter.h"
#include "DataTypes/GWBWorkGroup.h"
#include "GWBRuntimeModule.h"
#include "CVars.h"
#include "Stats.h"
void UGWBBudgeter::Reset() 
{
    FrameBudget = 0;
    LastFrameTime = 0;
    LastWorkTimestamp = 0;
    WorkUnitsCount = 0;
    WorkUnitsDoneThisCycle = 0;
    BudgetForFrameExceededTime = 0;
};

//
bool UGWBBudgeter::HasFrameBudgetBeenExceeded() const
{
    return FPlatformTime::Seconds() >= BudgetForFrameExceededTime;
}
bool UGWBBudgeter::HasCurrentGroupBudgetBeenExceeded() const
{
    //Group instance limit supersedes any instance guarantees
    if (HasCurrentGroupUnitBudgetBeenExceeded())
    {
        return true;
    }

    // have we run out of budget for the group
    if (HasCurrentGroupTimeBudgetBeenExceeded())
    {
        return true;
    }
    
    // static const double AvgInstanceTimeAlpha = 0.1; // always between 0 and 1, closer to 1 means new values have more weight
    // static const double AvgInstanceTimeInvAlpha = 1.0 - AvgInstanceTimeAlpha;
    //
    // // simple work telemetry recording
    // if (!GroupUnitWorkAverageDuration.Contains(Def.Id))
    // {
    //     GroupUnitWorkAverageDuration.Add(Def.Id, )
    // }
    //
    // // Check whether instance should be skipped this frame
    // // To reduce chances that we go over budget
    // // When any work has been done this frame
    // // Account for portion of average instance time
    // const bool HasAnyWorkBeenDoneThisFrame = WorkUnitsDoneThisCycle > 0;
    // const double StartInstanceTimeOffset = HasAnyWorkBeenDoneThisFrame ? WorkGroup.AverageUnitTime * 0.5 : 0.0;
    // if (StartInstanceTime + StartInstanceTimeOffset >= BudgetForGroupExceededTimestamp)
    // {
    //     UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UCPGGameBalancer::DoWorkForGroup -> Exceeded time for Group: %s, Instance %d (remaining: %d, global: %d), Start: %.3f"), *WorkGroup.Def.Id.ToString(), WorkInstanceIndex, WorkGroup.Instances.Num(), WorkUnitsCount, StartInstanceTime);
    //     //Don't need to check later instances in the queue because work instances are sorted by maxDelay,
    //     //so if we aren't required to perform this work, we won't be required to perform any other work in the queue
    //
    //     // However, to avoid skipping work in a group permanently, always decay average instance time
    //     WorkGroup.AverageUnitTime *= AvgInstanceTimeInvAlpha;
    //     return true;
    // }

    return false;
}
bool UGWBBudgeter::HasCurrentGroupTimeBudgetBeenExceeded() const
{
    return (FPlatformTime::Seconds() >= BudgetForGroupExceededTimestamp);
}
bool UGWBBudgeter::HasCurrentGroupUnitBudgetBeenExceeded() const
{
    return (MaxGroupWorkUnitsThisCycle > 0 && WorkUnitsForGroupDoneThisCycle >= MaxGroupWorkUnitsThisCycle);
}
float UGWBBudgeter::GetRemainingTimeInBudget() const
{
    return BudgetForFrameExceededTime - FPlatformTime::Seconds();
};
float UGWBBudgeter::GetRemainingTimeInBudgetForGroup() const
{
    return BudgetForGroupExceededTimestamp - FPlatformTime::Seconds();
};
//

void UGWBBudgeter::Start()
{
	LastFrameTime = FPlatformTime::Seconds();
	const double DeltaTime = LastFrameTime - LastWorkTimestamp;
	if (DeltaTime >= (double)CVarGWB_FrameInterval.GetValueOnGameThread()) 
    {
        LastWorkTimestamp = LastFrameTime;
        FrameBudget = (double)CVarGWB_FrameBudget.GetValueOnGameThread();
        BudgetForFrameExceededTime = LastWorkTimestamp + FrameBudget;
        WorkUnitsDoneThisCycle = 0;
    }
}
void UGWBBudgeter::StartGroup(const float GroupTimeBudget, const int MaxGroupWorkUnitsThisCycleIn)
{
    // TODO: loop though extensions, and call modify GroupBudget budget
    if (GroupTimeBudget > 0.0)
    {
        BudgetForGroupExceededTimestamp = FMath::Min(FPlatformTime::Seconds() + (double)GroupTimeBudget, BudgetForFrameExceededTime);
    } else
    {
        BudgetForGroupExceededTimestamp = BudgetForFrameExceededTime;
    }
    WorkUnitsForGroupDoneThisCycle = 0;
    MaxGroupWorkUnitsThisCycle = MaxGroupWorkUnitsThisCycleIn;
}
void UGWBBudgeter::OnWorkUnitScheduled() 
{
    WorkUnitsCount++;
    SET_DWORD_STAT(STAT_GameWorkBalancer_WorkCount, WorkUnitsCount);
};
void UGWBBudgeter::OnWorkUnitStarted() 
{
};
void UGWBBudgeter::OnWorkUnitComplete() 
{
    WorkUnitsCount--;
    WorkUnitsDoneThisCycle++;
    WorkUnitsForGroupDoneThisCycle++;
    SET_DWORD_STAT(STAT_GameWorkBalancer_WorkCount, WorkUnitsCount);
};

void UGWBBudgeter::RecordGroupTelemetry(const FName& GroupId, const double WorkDuration)
{
    // simple work telemetry recording
    if (!GroupUnitWorkAverageDuration.Contains(GroupId))
    {
        TCircularBuffer<double> TelemetryEvents{5};
        TelemetryEvents[0] = WorkDuration;
        GroupUnitWorkDurations.Add(GroupId,  TelemetryEvents);
        GroupUnitWorkAverageDuration.Add(GroupId, WorkDuration);
        GroupWorkUnitsRecorded.Add(GroupId, 1);
    } else
    {
        auto PastEventsBuffer = GroupUnitWorkDurations[GroupId];
        const auto Count = GroupWorkUnitsRecorded[GroupId];
        const auto Avg = GroupUnitWorkAverageDuration[GroupId];
        const auto PastWeight = (PastEventsBuffer.Capacity()-1) / PastEventsBuffer.Capacity();
        const auto NewEntryWeight = 1 / PastEventsBuffer.Capacity();
        GroupUnitWorkAverageDuration[GroupId] = NewEntryWeight * WorkDuration + PastWeight * Avg;
        PastEventsBuffer[Count] = WorkDuration;
        GroupWorkUnitsRecorded[GroupId] = Count + 1;
    }
}