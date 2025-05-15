// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GWBTimeSlicer.h"
#include "UObject/Object.h"
#include "GWBTimeSlicedLoopScope.generated.h"

// TODO: implement - BudgetedForLoop(0.1f, 10, Array,[](){ });

/**
 * 
 */
USTRUCT()
struct GWBRUNTIME_API FGWBTimeSlicedLoopScope
{
	GENERATED_BODY()
	
	FGWBTimeSlicedLoopScope()
			: Id(NAME_None)
			,WorldContextObject(nullptr)
	{
	}

	FGWBTimeSlicedLoopScope(const UObject* WorldContext, FName Id, double FrameTimeBudgetIn, uint32 WorkCountBudgetIn)
			: Id(Id)
			,WorldContextObject(WorldContext)
	{
		GlobalTimeSlicer = UGWBTimeSlicer::Get(WorldContext, Id);
		GlobalTimeSlicer.Get()
			->ConfigureTimeBudget(FrameTimeBudgetIn)
			->ConfigureWorkUnitCountBudget(WorkCountBudgetIn)
			->StartWork();
	}

	~FGWBTimeSlicedLoopScope()
	{
		GlobalTimeSlicer.Get()->EndWork();
	}

	/** Time Slicer Identifier. */
	FName Id;

	bool IsOverBudget() const { return GlobalTimeSlicer.Get()->HasBudgetBeenExceeded(); }
	bool IsOverFrameTimeBudget() const { return GlobalTimeSlicer.Get()->HasFrameBudgetBeenExceeded(); }
	bool IsOverUnitCountBudget() const { return GlobalTimeSlicer.Get()->HasWorkUnitCountBudgetBeenExceeded(); }

	TWeakObjectPtr<UGWBTimeSlicer> GetTimeSlicer() const { return GlobalTimeSlicer; };
	
private:
	
	TWeakObjectPtr<const UObject> WorldContextObject;
	TWeakObjectPtr<UGWBTimeSlicer> GlobalTimeSlicer;
};
