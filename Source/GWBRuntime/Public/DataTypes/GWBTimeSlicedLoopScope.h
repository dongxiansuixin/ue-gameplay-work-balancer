﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GWBTimeSlicer.h"
#include "UObject/Object.h"
#include "GWBTimeSlicedLoopScope.generated.h"

// TODO: implement - BudgetedForLoop(0.1f, 10, Array,[](){ });

/**
 * A convenience API to use time slicers in simple loops: allows you to control a for loop with an underlying hidden
 * `UGWBTimeSlicer` in order to distribute work across multiple frames. When it goes out of scope it resets the budget
 * back to max so it's a useful little tool to manage scoped work.
 *
 * EXAMPLE:
 * Let's say you have a global list of overlaps that have some expensive processing.
 * ```
 * TArray<FSomeOverlapResultThing> MyOverlapsToProcess;
 * ```
 *
 * You can now add one of the these scoped structs to a processing loop like so:
 * ```
 * OnTick(...)
 * {
 *	 static const float FrameBudget = 0.1f;
 *	 static const uint32 MaxCountAllowedPerFrame = 5;
 *   for (auto Overlap : MyOverlapsToProcess)
 *   {
 *     FGWBTimeSlicedLoopScope TimeSlicedWork(this, FName("OverlapsSlicer"), FrameBudget, MaxCountAllowedPerFrame);
 *     if (TimeSlicedWork.IsOverBudget()) break;
 *     DoSomeExpensiveProcessing(Overlap);
 *   }
 * }
 * ```
 * What this does is makes sure to break the loop until next tick once either FrameBudget is exceeded or the we do
 * more than MaxCountAllowedPerFrame worth of expensive processing function calls. If we have plenty of budget, it will
 * just reset back for the next tick once this struct goes out of scope.
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
