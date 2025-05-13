#pragma once

#include "CoreMinimal.h"
#include "GWBScheduler.generated.h"

DECLARE_DELEGATE_OneParam(FGWBOnSchedulerTick, float /* DeltaTime */);
DECLARE_DELEGATE(FGWBOnSchedulerStartWorkCycle);

/**
 * 
 */
UCLASS()
class GWBRUNTIME_API UGWBScheduler : public UObject
{
	GENERATED_BODY()
	
public:
	FGWBOnSchedulerStartWorkCycle StartWorkCycleDelegate;
	void Start();
	void Stop();

protected:
	FTimerHandle TickDelegateHandle;
	bool ScheduleNextFrame();
	void TickWork();
};
