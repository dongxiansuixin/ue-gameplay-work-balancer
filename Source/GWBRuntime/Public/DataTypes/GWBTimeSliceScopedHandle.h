// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GWBTimeSlicedLoopScope.h"
#include "Components/GWBTimeSlicer.h"
#include "GWBTimeSliceScopedHandle.generated.h"

/**
 * 
*/
USTRUCT()
struct GWBRUNTIME_API FGWBTimeSliceScopedHandle
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
	uint32 GetWorkUnitsCompleted() const { return UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)->GetCycleWorkUnitsCompleted(); }
	uint32 GetLastCycleTimestamp() const { return UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)->GetCycleLastTimestamp(); }

	FGWBTimeSlicedLoopScope NewTimeSlicedLoopScope(double FrameTimeBudgetIn, uint32 WorkCountBudgetIn) const
	{
		return FGWBTimeSlicedLoopScope(WorldContextObject.Get(), Id, FrameTimeBudgetIn, WorkCountBudgetIn);
	}

private:
	
	TWeakObjectPtr<const UObject> WorldContextObject;
};
