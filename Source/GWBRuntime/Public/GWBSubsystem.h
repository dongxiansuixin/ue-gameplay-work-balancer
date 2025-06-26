// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GWBSubsystem.generated.h"

class UGWBManager;
class UGWBTimeSlicer;

/**
 * Use the GWB subsystem to access the `UGWBManager` singleton via `GEngine->GetEngineSubsystem<UGWBSubsystem>()->GetManager()`.
 * Other internal tools also use the subsystem to manage a global array of Time Slicers so that we can balance work across frames
 * and stack frames.
 */
UCLASS()
class GWBRUNTIME_API UGWBSubsystem : public UEngineSubsystem 
{
	GENERATED_BODY()
public:
	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	/** The primary singleton used to manage work balancing. */
	UGWBManager* GetManager() const;

	/** Get or create a time slicer for the identifier. At the moment, the timeslicer lives forever once created (this should be improved somehow). */
	UGWBTimeSlicer* GetTimeSlicer(const FName& Id);
	
private:

	/** The main singleton used to manage work balancing. This object does the meat and potatoes of the whole system. */
	UPROPERTY() UGWBManager* Manager;

	/** We keep a global stateful list of time slicers so we can use them across frames. */
	UPROPERTY() TMap<FName, UGWBTimeSlicer*> TimeSlicers;
};
