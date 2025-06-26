#pragma once

#include "CoreMinimal.h"
#include "GWBScheduler.generated.h"

DECLARE_DELEGATE_OneParam(FGWBOnSchedulerTick, float /* DeltaTime */);
DECLARE_DELEGATE(FGWBOnSchedulerStartWorkCycle);

/**
 * Manages the scheduling of the work loop. Used by `GWBManager` to schedule itself on tick.
 */
UCLASS()
class GWBRUNTIME_API UGWBScheduler : public UObject
{
	GENERATED_BODY()
	
public:
	/** Use this delegate to start processing a work batch. */
	FGWBOnSchedulerStartWorkCycle StartWorkCycleDelegate;

	/** Starts scheduling on tick and firing the schedule delegates. */
	void Start();
	
	/** Stops scheduling and delegates should stop firing. */
	void Stop();

protected:
	FTimerHandle TickDelegateHandle;
	bool ScheduleNextFrame();
	void TickWork();
};
