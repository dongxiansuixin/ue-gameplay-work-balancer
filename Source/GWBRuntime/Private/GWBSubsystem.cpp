// Fill out your copyright notice in the Description page of Project Settings.


#include "GWBSubsystem.h"
#include "GWBManager.h"
#include "Components/GWBTimeSlicer.h"

void UGWBSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Manager = NewObject<UGWBManager>();
}

void UGWBSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UGWBManager* UGWBSubsystem::GetManager() const
{
	return Manager;
}

UGWBTimeSlicer* UGWBSubsystem::GetTimeSlicer(const FName& Id)
{
	if (!TimeSlicers.Contains(Id))
	{
		TimeSlicers.Add(Id, NewObject<UGWBTimeSlicer>());
		TimeSlicers[Id]->Init();
	}

	return TimeSlicers[Id];
}

