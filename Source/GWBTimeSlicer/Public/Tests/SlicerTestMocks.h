#pragma once

#include "CoreMinimal.h"
#include "Utils/GWBLoopUtils.h"

#include "SlicerTestMocks.generated.h"

#pragma region Test Helpers

/**
 * Test helper object for Blueprint delegate binding in loop utils tests
 */
UCLASS()
class GWBTIMESLICER_API UGWBLoopUtilsTestHelper : public UObject
{
	GENERATED_BODY()
public:
	int32 ProcessedCount = 0;
	bool bShouldBreak = false;
	int32 BreakAtCount = -1;
	float SleepDuration = 0.0f;
	TFunction<void(FBudgetedLoopHandle&)> CustomCallback;
	
	UFUNCTION()
	void ProcessWorkUnit(FBudgetedLoopHandle& LoopHandle);
	
	void ResetCounter();
	void SetBreakAtCount(int32 Count);
	void SetSleepDuration(float Duration);
	void SetCustomCallback(TFunction<void(FBudgetedLoopHandle&)> Callback);
};

/**
 * Simple generic world context object for testing TimeSlicer functionality
 * without depending on the GWBRuntime module
 */
UCLASS()
class GWBTIMESLICER_API UGWBTestWorldContext : public UObject
{
	GENERATED_BODY()
public:
	// Simple world context object - no special functionality needed
	// The TimeSlicer subsystem will handle the actual work
};

#pragma endregion Test Helpers