#pragma once

#include "GWBWorkOptions.generated.h"

USTRUCT(BlueprintType)
struct GWBRUNTIME_API FGWBWorkOptions
{
	GENERATED_BODY()
	
	FGWBWorkOptions()
		: Priority(0)
		, MaxDelay(0.f)
		, MaxNumSkippedFrames(0)
		, bAddToFrontOfPriorityQueue(false)
		, bDeferToNextFrame(false)
	{
	}

	FGWBWorkOptions(int32 InPriority, float InMaxDelay = 0.f, int32 InMaxNumSkippedFrames = 0, bool bInAddToFrontOfPriorityQueue = false, bool bInDeferToNextFrame = false)
		: Priority(InPriority)
		, MaxDelay(InMaxDelay)
		, MaxNumSkippedFrames(InMaxNumSkippedFrames)
		, bAddToFrontOfPriorityQueue(bInAddToFrontOfPriorityQueue)
		, bDeferToNextFrame(bInDeferToNextFrame)
	{
	}

	static const FGWBWorkOptions EmptyOptions;

	/**
	 * Priority of this work unit within its group.
	 * NOTE: This setting is relative to the category, i.e. it will never allow work to prioritize outside of its own category.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	int32 Priority;

	/**
	 * Maximum delay in seconds before this work must be done (i.e. 0.001 = 1ms). When <= 0, this setting is ignored.
	 * NOTES:
	 * - any value > 0 will cause this work to be prioritized above all work with no maximum delay.
	 * - lowest remaining delay > 0 is prioritized above pure priority setting.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default", meta = (UIMin = 0, ClampMin = 0))
	float MaxDelay;

	/** Maximum number of frames allowed to skip before work must be done. When <=0, this setting is ignored. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	int32 MaxNumSkippedFrames;

	/** Whether this work should be added to the front or back of the units in the work queue with the same priority. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	bool bAddToFrontOfPriorityQueue;

	/** Whether this unit of work should be deferred and may NOT be executed immediately if scheduled while already working on the same work group. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Default")
	bool bDeferToNextFrame;
};