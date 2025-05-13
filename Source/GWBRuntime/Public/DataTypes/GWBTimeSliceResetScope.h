// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/GWBTimeSlicer.h"
#include "GWBTimeSliceResetScope.generated.h"

/**
 * 
*/
USTRUCT()
struct GWBRUNTIME_API FGWBTimeSliceResetScope
{
	GENERATED_BODY()
	
	FGWBTimeSliceResetScope()
			: Id(NAME_None)
			,WorldContextObject(nullptr)
	{
	}

	FGWBTimeSliceResetScope(const UObject* WorldContext, const FName Id)
			: Id(Id)
			,WorldContextObject(WorldContext)
	{
	}

	~FGWBTimeSliceResetScope()
	{
		if (WorldContextObject.IsValid())
		{
			UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)->Reset();	
		}
	}

	/** Time Slicer Identifier. */
	FName Id;

	bool IsOverBudget() const { return UGWBTimeSlicer::Get(WorldContextObject.Get(), Id)->HasBudgetBeenExceeded(); }

private:
	
	TWeakObjectPtr<const UObject> WorldContextObject;
};
