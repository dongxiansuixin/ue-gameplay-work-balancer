#pragma once

#include "GWBWorkOptions.h"
#include "GWBWorkUnit.generated.h"

struct FGWBWorkUnitHandle;

DECLARE_DELEGATE_TwoParams(FGWBOnDoWorkDelegate, float /*, DeltaTime*/, const FGWBWorkUnitHandle& /*, WorkUnit*/);
DECLARE_DELEGATE(FGWBAbortWorkDelegate);

USTRUCT()
struct GWBRUNTIME_API FGWBWorkUnitCallback
{
	GENERATED_BODY()
	
	/** callback when work should be done with delta time since scheduled. */
	FGWBOnDoWorkDelegate WorkCallback;

	/** callback when work should be aborted. */
	FGWBAbortWorkDelegate AbortCallback;
};

USTRUCT(BlueprintType)
struct GWBRUNTIME_API FGWBWorkUnit
{
	GENERATED_BODY()

	FGWBWorkUnit()
		: ScheduledTimestamp(0)
		  , bHasCompletedWork(false)
		  , bIsAborted(false)
	{
		CallbackHandle = MakeShared<FGWBWorkUnitCallback>();
	}
	FGWBWorkUnit(const FGWBWorkOptions& InOptions, double InTimeScheduled)
		: Options(InOptions)
		, ScheduledTimestamp(InTimeScheduled)
		, bHasCompletedWork(false)
		, bIsAborted(false)
	{
		CallbackHandle = MakeShared<FGWBWorkUnitCallback>();
	}

	/** custom options used to schedule this unit of work. */
	UPROPERTY()
	FGWBWorkOptions Options;

	/** the moment in time this work was scheduled. */
	UPROPERTY()
	double ScheduledTimestamp;

	/** callback when work should be done with delta time since scheduled. */
	TSharedPtr<FGWBWorkUnitCallback> CallbackHandle;
	
	FORCEINLINE int32 GetId() const { return 0; }
	FORCEINLINE bool HasWork() const { return !bHasCompletedWork || bIsAborted; }
	FORCEINLINE bool HasCompletedWork() const { return bHasCompletedWork; }
	FORCEINLINE void MarkCompleted() const { bHasCompletedWork = true; }
	FORCEINLINE void MarkAborted() const { bIsAborted = true; }

	FORCEINLINE FGWBOnDoWorkDelegate& GetWorkCallback() const { return CallbackHandle.Get()->WorkCallback; }
	FORCEINLINE FGWBAbortWorkDelegate& GetAbortCallback() const { return CallbackHandle.Get()->AbortCallback; }

protected:
    mutable bool bHasCompletedWork;
    mutable bool bIsAborted;
};
