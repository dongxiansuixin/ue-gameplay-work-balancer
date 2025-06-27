#pragma once

#include "CoreMinimal.h"
#include "GWBWorkUnit.h"
#include "GWBWorkUnitHandle.generated.h"

DECLARE_DYNAMIC_DELEGATE_OneParam(FGWBBlueprintWorkDelegate, float, DeltaTime);

/**
 * Returned by the `GWBManager` when you schedule work, this handle allows you to provide the callback for the code you
 * want balanced across frames via either `OnHandleWork([](){..}) or via the delegate in `GetWorkCallback()`.
 *
 * The lambda or delegate callback you provide is the actual work that will be done when there is room in the budget
 * managed by the `GWBManager`.
 */
USTRUCT(BlueprintType)
struct GWBRUNTIME_API FGWBWorkUnitHandle
{
	GENERATED_BODY()

	// Default constructor
	FGWBWorkUnitHandle(): Id(0), bShouldAutoFire(false)
	{
	}

	FGWBWorkUnitHandle(const FGWBWorkUnit& WorkUnit): bShouldAutoFire(false)
	{
		WorkUnitCallbackHandle = WorkUnit.CallbackHandle;
		Id = WorkUnit.GetId();
	}

	/** Provide the function that will do work when there is room in the budget. */
	void OnHandleWork(TFunction<void(const float DeltaTime)> DispatchOnDoWork) const;

	/** Get the delegate that will broadcast when there is room in the budget to do some work. */
	FORCEINLINE FGWBOnDoWorkDelegate& GetWorkCallback() const { return WorkUnitCallbackHandle.Get()->WorkCallback; }

	/** Get the delegate that will broadcast when this work unit was aborted. */
	FORCEINLINE FGWBAbortWorkDelegate& GetAbortCallback() const { return WorkUnitCallbackHandle.Get()->AbortCallback; }

	/**
	 * A handle that does nothing and immediately fires it's callbacks.
	 * This is what you get when you schedule work while the
	 * whole system is disabled via the CVar `gwb.enabled`
	 */
	static FGWBWorkUnitHandle PassthroughHandle()
	{
		FGWBWorkUnitHandle Handle;
		Handle.bShouldAutoFire = true;
		return Handle;
	}
	
	FORCEINLINE int32 GetId() const { return Id; }
	
protected:
	int32 Id;
	bool bShouldAutoFire;
	TSharedPtr<FGWBWorkUnitCallback> WorkUnitCallbackHandle;
};
