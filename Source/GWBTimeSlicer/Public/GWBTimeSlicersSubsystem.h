#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "GWBTimeSlicersSubsystem.generated.h"

class UGWBTimeSlicer;

/**
 * Subsystem responsible for managing a global collection of Time Slicers.
 * This subsystem maintains a stateful list of time slicers so they can be used across frames
 * to balance work across frames and stack frames.
 */
UCLASS()
class GWBTIMESLICER_API UGWBTimeSlicersSubsystem : public UEngineSubsystem 
{
	GENERATED_BODY()
public:
	// Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// End USubsystem

	/** Get or create a time slicer for the identifier. At the moment, the timeslicer lives forever once created (this should be improved somehow). */
	UGWBTimeSlicer* GetTimeSlicer(const FName& Id);
	
private:
	/** We keep a global stateful list of time slicers so we can use them across frames. */
	UPROPERTY() TMap<FName, UGWBTimeSlicer*> TimeSlicers;
};