#include "GWBTimeSlicersSubsystem.h"
#include "Components/GWBTimeSlicer.h"

void UGWBTimeSlicersSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGWBTimeSlicersSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UGWBTimeSlicer* UGWBTimeSlicersSubsystem::GetTimeSlicer(const FName& Id)
{
	if (!TimeSlicers.Contains(Id))
	{
		TimeSlicers.Add(Id, NewObject<UGWBTimeSlicer>());
		TimeSlicers[Id]->Init();
	}

	return TimeSlicers[Id];
}