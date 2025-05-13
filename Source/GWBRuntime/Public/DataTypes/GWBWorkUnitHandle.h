#pragma once

#include "CoreMinimal.h"
#include "GWBWorkUnit.h"
#include "GWBWorkUnitHandle.generated.h"

USTRUCT(BlueprintType)
struct GWBRUNTIME_API FGWBWorkUnitHandle
{
	GENERATED_BODY()

	// Default constructor
	FGWBWorkUnitHandle(): bShouldAutoFire(false)
	{
	}
	
	FGWBWorkUnitHandle(FGWBWorkUnit& WorkUnit): bShouldAutoFire(false)
	{
		WorkUnitCallbackHandle = WorkUnit.CallbackHandle;
	}

	
	void OnHandleWork(TFunction<void(const float DeltaTime)> DispatchOnDoWork) const;
	FORCEINLINE FGWBOnDoWorkDelegate& GetWorkCallback() const { return WorkUnitCallbackHandle.Get()->WorkCallback; }
	FORCEINLINE FGWBAbortWorkDelegate& GetAbortCallback() const { return WorkUnitCallbackHandle.Get()->AbortCallback; }
	static FGWBWorkUnitHandle PassthroughHandle()
	{
		FGWBWorkUnitHandle Handle;
		Handle.bShouldAutoFire = true;
		return Handle;
	}
protected:
	bool bShouldAutoFire;
	TSharedPtr<FGWBWorkUnitCallback> WorkUnitCallbackHandle;
};
