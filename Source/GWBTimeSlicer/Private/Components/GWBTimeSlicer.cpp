// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/GWBTimeSlicer.h"

#include "GWBTimeSlicersSubsystem.h"
#include "GWBTimeSlicerModule.h"

UGWBTimeSlicer::UGWBTimeSlicer()
{
	LastResetTimestamp = FPlatformTime::Seconds();
}

void UGWBTimeSlicer::Init()
{
	UnitWorkDurations = TCircularBuffer<double>(5);
}

UGWBTimeSlicer* UGWBTimeSlicer::Get(const UObject* WorldContextObject, FName Id)
{
	UGWBTimeSlicersSubsystem* Subsystem = GEngine->GetEngineSubsystem<UGWBTimeSlicersSubsystem>();
	return Subsystem->GetTimeSlicer(Id);
}

void UGWBTimeSlicer::Reset()
{
	CycleWorkUnitsCompleted = 0;
	CycleLastTimestamp = 0;
	LastResetTimestamp = FPlatformTime::Seconds();
	UE_LOG(Log_GameplayWorkTimeSlicer, VeryVerbose, TEXT("UGWBTimeSlicer::Reset -> Remaining Budget: %f)"), GetRemainingTimeInBudget());
}

void UGWBTimeSlicer::StartWork()
{
	CycleLastTimestamp = FPlatformTime::Seconds();
}

void UGWBTimeSlicer::EndWork()
{
	CycleWorkUnitsCompleted++;
	RecordTelemetry(FPlatformTime::Seconds() - CycleLastTimestamp);
	UE_LOG(Log_GameplayWorkTimeSlicer, VeryVerbose, TEXT("UGWBTimeSlicer::EndWork -> Remaining Budget: %f)"), GetRemainingTimeInBudget());
}

bool UGWBTimeSlicer::HasBudgetBeenExceeded() const
{
	return HasWorkUnitCountBudgetBeenExceeded() || HasFrameBudgetBeenExceeded();
}

bool UGWBTimeSlicer::HasFrameBudgetBeenExceeded() const
{
	const auto RemainingTime = GetRemainingTimeInBudget();
	const bool IsOutOfTime = RemainingTime <= DOUBLE_SMALL_NUMBER;
	return FrameTimeBudget >= 0 ? IsOutOfTime : false;
}

bool UGWBTimeSlicer::HasWorkUnitCountBudgetBeenExceeded() const
{
	return WorkUnitCountBudget > 0 ? GetRemainingWorkUnitCountBudget() <= 0 : false;
}

float UGWBTimeSlicer::GetRemainingTimeInBudget() const
{
	return GetFrameBudgetExceededTimestamp() - FPlatformTime::Seconds();
}

double UGWBTimeSlicer::GetFrameBudgetExceededTimestamp() const
{
	const auto Value = LastResetTimestamp + FrameTimeBudget;
	return Value;
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
