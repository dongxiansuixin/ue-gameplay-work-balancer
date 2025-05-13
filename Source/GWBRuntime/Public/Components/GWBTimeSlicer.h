// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GWBTimeSlicer.generated.h"

/**
 * 
 */
UCLASS()
class GWBRUNTIME_API UGWBTimeSlicer : public UObject
{
	GENERATED_BODY()
	
public:

	void Init();

	/**
	 * @brief get global singleton slicer by Id
	 * @param Id 
	 * @return global time slicer for the provided Id
	 */
	UFUNCTION()
	static UGWBTimeSlicer* Get(const UObject* WorldContextObject, FName Id);

	// <api>
	void Reset();
	void StartWork();
	void EndWork();
	bool HasBudgetBeenExceeded() const;
	bool HasFrameBudgetBeenExceeded() const;
	bool HasWorkUnitCountBudgetBeenExceeded() const;
	float GetRemainingTimeInBudget() const;
	uint32 GetRemainingWorkUnitCountBudget() const;
	// </api>

	// <builder-methods>
	UGWBTimeSlicer* ConfigureTimeBudget(double FrameTimeBudgetIn);
	UGWBTimeSlicer* ConfigureWorkUnitCountBudget(uint32 WorkUnitCountBudgetIn);
	// </builder-methods>

private:

	// <config>
	double FrameTimeBudget; // this is a value in seconds
	uint32 WorkUnitCountBudget; // how many units we're allowed to do
	// </config>

	// <state>
	uint32 CycleWorkUnitsDone = 0;
	double CycleLastTimestamp; // this is a specific point in platform time
	double FrameBudgetExceededTimestamp; // this is a specific point in platform time
	// </state>

	// <telemetry>
	double UnitWorkAverageDuration;
	uint32 WorkUnitsRecorded;
	TCircularBuffer<double> UnitWorkDurations{5};
	void RecordTelemetry(const double WorkDuration);
	// </telemetry>
};
