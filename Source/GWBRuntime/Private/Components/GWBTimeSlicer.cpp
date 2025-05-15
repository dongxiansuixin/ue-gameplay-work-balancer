// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GWBTimeSlicer.h"

#include "GWBSubsystem.h"

void UGWBTimeSlicer::Init()
{
	UnitWorkDurations = TCircularBuffer<double>(5);
}

UGWBTimeSlicer* UGWBTimeSlicer::Get(const UObject* WorldContextObject, FName Id)
{
	UGWBSubsystem* Subsystem = GEngine->GetEngineSubsystem<UGWBSubsystem>();
	return Subsystem->GetTimeSlicer(Id);
}

void UGWBTimeSlicer::Reset()
{
	CycleWorkUnitsCompleted = 0;
	CycleLastTimestamp = 0;
	FrameBudgetExceededTimestamp = FPlatformTime::Seconds() + FrameTimeBudget;
}

void UGWBTimeSlicer::StartWork()
{
	CycleLastTimestamp = FPlatformTime::Seconds();
}

void UGWBTimeSlicer::EndWork()
{
	CycleWorkUnitsCompleted++;
	RecordTelemetry(FPlatformTime::Seconds() - CycleLastTimestamp);
}

bool UGWBTimeSlicer::HasBudgetBeenExceeded() const
{
	return HasWorkUnitCountBudgetBeenExceeded() || HasFrameBudgetBeenExceeded();
}

bool UGWBTimeSlicer::HasFrameBudgetBeenExceeded() const
{
	return GetRemainingTimeInBudget() > 0;
}

bool UGWBTimeSlicer::HasWorkUnitCountBudgetBeenExceeded() const
{
	return GetRemainingWorkUnitCountBudget() > 0;
}

float UGWBTimeSlicer::GetRemainingTimeInBudget() const
{
	return FrameBudgetExceededTimestamp - FPlatformTime::Seconds();
}

uint32 UGWBTimeSlicer::GetRemainingWorkUnitCountBudget() const
{
	return WorkUnitCountBudget - CycleWorkUnitsCompleted;
}

UGWBTimeSlicer* UGWBTimeSlicer::ConfigureTimeBudget(double FrameTimeBudgetIn)
{
	FrameTimeBudget = FrameTimeBudgetIn;
	return this;
}

UGWBTimeSlicer* UGWBTimeSlicer::ConfigureWorkUnitCountBudget(uint32 WorkUnitCountBudgetIn)
{
	WorkUnitCountBudget = WorkUnitCountBudgetIn;
	return this;
}

void UGWBTimeSlicer::RecordTelemetry(const double WorkDuration)
{
	const auto PastWeight = (UnitWorkDurations.Capacity()-1) / UnitWorkDurations.Capacity();
	const auto NewEntryWeight = 1 / UnitWorkDurations.Capacity();
	UnitWorkAverageDuration = NewEntryWeight * WorkDuration + PastWeight * UnitWorkAverageDuration;
	UnitWorkDurations[WorkUnitsRecorded] = WorkDuration;
	WorkUnitsRecorded += 1;
}
