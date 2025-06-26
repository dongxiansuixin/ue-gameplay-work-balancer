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

/**
 * @brief The stateful record of a unit of work that needs to be done. As a user you should use `FGWBWorkUnitHandle`.
 * This is intended to be an internal record and not really something for end-users to need to deal with (copying it around etc.)
 * End users of the system should rely on `FGWBWorkUnitHandle` which provides a way to access the delegates you need
 * when work is ready to be done or when the underlying work unit is aborted or fails.
 */
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
		Id = FMath::Rand32();
	}
	FGWBWorkUnit(const FGWBWorkOptions& InOptions, double InTimeScheduled)
		: Options(InOptions)
		, ScheduledTimestamp(InTimeScheduled)
		, bHasCompletedWork(false)
		, bIsAborted(false)
	{
		CallbackHandle = MakeShared<FGWBWorkUnitCallback>();
		Id = FMath::Rand32();
	}

	/** custom options used to schedule this unit of work. */
	UPROPERTY()
	FGWBWorkOptions Options;

	/** the moment in time this work was scheduled. */
	UPROPERTY()
	double ScheduledTimestamp;

	/** callback when work should be done with delta time since scheduled. */
	TSharedPtr<FGWBWorkUnitCallback> CallbackHandle;
	
	FORCEINLINE int32 GetId() const { return Id; }
	FORCEINLINE bool HasWork() const { return !bHasCompletedWork || bIsAborted; }
	FORCEINLINE bool HasCompletedWork() const { return bHasCompletedWork; }
	FORCEINLINE void MarkCompleted() const { bHasCompletedWork = true; }
	FORCEINLINE void MarkAborted() const { bIsAborted = true; }
	
	// Get effective priority including any runtime adjustments
	FORCEINLINE int32 GetEffectivePriority() const { return Options.Priority + PriorityOffset; }

	FORCEINLINE FGWBOnDoWorkDelegate& GetWorkCallback() const { return CallbackHandle.Get()->WorkCallback; }
	FORCEINLINE FGWBAbortWorkDelegate& GetAbortCallback() const { return CallbackHandle.Get()->AbortCallback; }
	
	// Runtime priority adjustment for this work unit
	int32 PriorityOffset = 0;

protected:
	int32 Id;
    mutable bool bHasCompletedWork;
    mutable bool bIsAborted;
};
