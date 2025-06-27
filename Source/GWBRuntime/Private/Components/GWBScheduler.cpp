#include "Components/GWBScheduler.h"
#include "GWBRuntimeModule.h"


void UGWBScheduler::Start() 
{
	ScheduleNextFrame();
};
bool UGWBScheduler::ScheduleNextFrame() 
{
	if (TickDelegateHandle.IsValid()) return false;
	
	UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBScheduler::ScheduleNextFrame"));

	if (const UWorld* World = GetWorld())
	{
		const auto Del = FTimerDelegate::CreateUObject(this, &UGWBScheduler::TickWork);
		TickDelegateHandle = World->GetTimerManager().SetTimerForNextTick(Del);
		return true;
	}
	
	return false;
};
void UGWBScheduler::TickWork() 
{
	if (!TickDelegateHandle.IsValid()) return;
	TickDelegateHandle.Invalidate();
	StartWorkCycleDelegate.ExecuteIfBound();
};
void UGWBScheduler::Stop() 
{
	if (!TickDelegateHandle.IsValid()) return;
	
	UE_LOG(Log_GameplayWorkBalancer, VeryVerbose, TEXT("UGWBScheduler::Stop"));

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickDelegateHandle);
	}
	else
	{
		TickDelegateHandle.Invalidate();
	}
};