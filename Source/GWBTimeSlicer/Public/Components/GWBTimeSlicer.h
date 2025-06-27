// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GWBTimeSlicer.generated.h"

/**
 * Tracks how much time work has taken and provides data on remaining budget as well as telemetry for average work duration.
 * You don't need to use the time slicer directly, it's intended intended to be used with a simplified scope API (`FGWBTimeSlicedLoopScope`).
 * Check the example in FGWBTimeSlicedLoopScope.h to learn the basic approach.
 */
UCLASS()
class GWBTIMESLICER_API UGWBTimeSlicer : public UObject
{
	GENERATED_BODY()

	UGWBTimeSlicer();
	
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
	double GetFrameBudgetExceededTimestamp() const;
	uint32 GetRemainingWorkUnitCountBudget() const;
	FORCEINLINE uint32 GetCycleWorkUnitsCompleted() const { return CycleWorkUnitsCompleted; };
	FORCEINLINE double GetCycleLastTimestamp() const { return CycleLastTimestamp; };
	FORCEINLINE double GetLastResetTimestamp() const { return LastResetTimestamp; };
	// </api>

	// <builder-methods>
	UGWBTimeSlicer* ConfigureTimeBudget(double FrameTimeBudgetIn);
	UGWBTimeSlicer* ConfigureWorkUnitCountBudget(uint32 WorkUnitCountBudgetIn);
	// </builder-methods>

	// <getters>
	double GetFrameTimeBudget() const { return FrameTimeBudget; }
	double GetWorkUnitCountBudget() const { return WorkUnitCountBudget; }
	// </getters>

private:

	// <config>
	double FrameTimeBudget; // this is a value in seconds
	int32 WorkUnitCountBudget; // how many units we're allowed to do (-1 means infinite)
	// </config>

	// <state>
	uint32 CycleWorkUnitsCompleted = 0;
	double CycleLastTimestamp; // this is a specific point in platform time
	double LastResetTimestamp; // specific point in time the slicer has been reset to 0 so we have full budget
	// </state>

	// <telemetry>
	double UnitWorkAverageDuration;
	uint32 WorkUnitsRecorded;
	TCircularBuffer<double> UnitWorkDurations{5};
	void RecordTelemetry(const double WorkDuration);
	// </telemetry>
};
