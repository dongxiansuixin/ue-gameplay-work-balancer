// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GWBSubsystem.generated.h"

class UGWBManager;

/**
 * Use the GWB subsystem to access the `UGWBManager` singleton via `GEngine->GetEngineSubsystem<UGWBSubsystem>()->GetManager()`.
 * For TimeSlicer management, use UGWBTimeSlicersSubsystem instead.
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
	
private:

	/** The main singleton used to manage work balancing. This object does the meat and potatoes of the whole system. */
	UPROPERTY() UGWBManager* Manager;
};
