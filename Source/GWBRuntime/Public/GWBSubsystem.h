// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GWBSubsystem.generated.h"

class UGWBManager;
class UGWBTimeSlicer;
/**
 * 
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

	UGWBManager* GetManager() const;
	UGWBTimeSlicer* GetTimeSlicer(const FName& Id);
	
private:

	UPROPERTY()
	UGWBManager* Manager;

	UPROPERTY()
	TMap<FName, UGWBTimeSlicer*> TimeSlicers;
};
