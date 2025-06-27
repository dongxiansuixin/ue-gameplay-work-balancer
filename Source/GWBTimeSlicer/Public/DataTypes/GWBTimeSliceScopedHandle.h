// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GWBTimeSlicedLoopScope.h"
#include "Components/GWBTimeSlicer.h"
#include "GWBTimeSliceScopedHandle.generated.h"

/**
 * @brief Manages resetting time slicer used budgets back to 0. Define one of these inside a function but outside a loop and when it goes out of scope it resets the time slice budgets.
 * @see `FGWBTimeSlicedLoopScope`
 */
USTRUCT()
struct GWBTIMESLICER_API FGWBTimeSliceScopedHandle
{
	GENERATED_BODY()
	
	FGWBTimeSliceScopedHandle()
			: Id(NAME_None)
			,WorldContextObject(nullptr)
	{
	}

	FGWBTimeSliceScopedHandle(const UObject* WorldContext, const FName Id)
			: Id(Id)
			,WorldContextObject(WorldContext)
	{
		UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)
			->Reset();	
	}

	FGWBTimeSliceScopedHandle(const UObject* WorldContext, const FName Id, double FrameTimeBudgetIn, uint32 WorkCountBudgetIn)
			: Id(Id)
			,WorldContextObject(WorldContext)
	{
		UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)
			->ConfigureTimeBudget(FrameTimeBudgetIn)
			->ConfigureWorkUnitCountBudget(WorkCountBudgetIn)
			->Reset();	
	}

	~FGWBTimeSliceScopedHandle()
	{
		if (WorldContextObject.IsValid())
		{
			UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)->Reset();	
		}
	}

	/** Time Slicer Identifier. */
	FName Id;

	bool IsOverBudget() const { return UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)->HasBudgetBeenExceeded(); }
	double GetRemainingTimeInBudget() const { return UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)->GetRemainingTimeInBudget(); }
	uint32 GetWorkUnitsCompleted() const { return UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)->GetCycleWorkUnitsCompleted(); }
	double GetLastCycleTimestamp() const { return UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)->GetCycleLastTimestamp(); }
	double GetLastResetTimestamp() const { return UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)->GetLastResetTimestamp(); }

	FGWBTimeSlicedLoopScope StartLoopScope() const
	{
		return FGWBTimeSlicedLoopScope(WorldContextObject.Get(), Id);
	}
	FGWBTimeSlicedLoopScope StartLoopScopeWithCustomBudget(double FrameTimeBudgetIn, uint32 WorkCountBudgetIn) const
	{
		return FGWBTimeSlicedLoopScope(WorldContextObject.Get(), Id, FrameTimeBudgetIn, WorkCountBudgetIn);
	}

private:
	
	TWeakObjectPtr<const UObject> WorldContextObject;
};
