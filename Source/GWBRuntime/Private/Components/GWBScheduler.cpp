#include "Components/GWBScheduler.h"


void UGWBScheduler::Start() 
{
	if (ScheduleNextFrame())
	{
		return;
	}
};
bool UGWBScheduler::ScheduleNextFrame() 
{
	if (TickDelegateHandle.IsValid()) return false;

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

	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TickDelegateHandle);
	}
	else
	{
		TickDelegateHandle.Invalidate();
	}
};