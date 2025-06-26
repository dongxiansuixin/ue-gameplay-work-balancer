#include "Tests/SlicerTestMocks.h"
#include "HAL/PlatformProcess.h"

// Implementation of UGWBLoopUtilsTestHelper methods
void UGWBLoopUtilsTestHelper::ProcessWorkUnit(FBudgetedLoopHandle& LoopHandle)
{
	ProcessedCount++;
	
	// Execute custom callback if provided
	if (CustomCallback)
	{
		CustomCallback(LoopHandle);
	}
	
	// Check for break condition
	if (bShouldBreak || (BreakAtCount > 0 && ProcessedCount >= BreakAtCount))
	{
		LoopHandle.Break();
	}
	
	// Add sleep if specified
	if (SleepDuration > 0.0f)
	{
		FPlatformProcess::Sleep(SleepDuration);
	}
}

void UGWBLoopUtilsTestHelper::ResetCounter()
{
	ProcessedCount = 0;
	bShouldBreak = false;
	BreakAtCount = -1;
	SleepDuration = 0.0f;
	CustomCallback = nullptr;
}

void UGWBLoopUtilsTestHelper::SetBreakAtCount(int32 Count)
{
	BreakAtCount = Count;
}

void UGWBLoopUtilsTestHelper::SetSleepDuration(float Duration)
{
	SleepDuration = Duration;
}

void UGWBLoopUtilsTestHelper::SetCustomCallback(TFunction<void(FBudgetedLoopHandle&)> Callback)
{
	CustomCallback = Callback;
}