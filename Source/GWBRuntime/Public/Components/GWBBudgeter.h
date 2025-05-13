#pragma once

#include "CoreMinimal.h"
#include "Containers/RingBuffer.h"
#include "UObject/Object.h"
#include "GWBBudgeter.generated.h"

struct FGWBWorkGroupDefinition;

/**
 * 
 */
UCLASS()
class GWBRUNTIME_API UGWBBudgeter : public UObject
{
	GENERATED_BODY()
	
public:

    double FrameBudget; // this is a value in seconds
    uint32 WorkUnitsCount; // total number of work units in all queues across all groups
    uint32 WorkUnitsDoneThisCycle = 0;
	uint32 WorkUnitsForGroupDoneThisCycle = 0;
    double LastFrameTime; // this is a specific point in platform time
    double LastWorkTimestamp; // this is a specific point in platform time
    double BudgetForFrameExceededTime; // this is a specific point in platform time
    double BudgetForGroupExceededTimestamp; // this is a specific point in platform time

	uint32 MaxGroupWorkUnitsThisCycle = 0;

	// <telemetry>
	TMap<FName,double> GroupUnitWorkAverageDuration;
	TMap<FName,uint32> GroupWorkUnitsRecorded;
	TMap<FName,TCircularBuffer<double>> GroupUnitWorkDurations;
	// </telemetry>
	
	//
	float GetRemainingTimeInBudget() const;
	float GetRemainingTimeInBudgetForGroup() const;
	bool HasCurrentGroupBudgetBeenExceeded() const;
	bool HasCurrentGroupTimeBudgetBeenExceeded() const;
	bool HasCurrentGroupUnitBudgetBeenExceeded() const;
	bool HasFrameBudgetBeenExceeded() const;
	//

	void Reset();
	void Start();
    void StartGroup(const float GroupTimeBudget, const int MaxGroupWorkUnitsThisCycleIn);
    void OnWorkUnitScheduled();
    void OnWorkUnitStarted();
    void OnWorkUnitComplete();
	void RecordGroupTelemetry(const FName& GroupId, const double WorkDuration);
};
