#include "Utils/GWBLoopUtils.h"
#include "DataTypes/GWBTimeSlicedLoopScope.h"

// Static member definition
TMap<uint32, bool> UGWBLoopUtilsBlueprintLibrary::BlueprintBreakStates;

// FBudgetedLoopHandle implementation
void FBudgetedLoopHandle::Break()
{
    bShouldBreak = true;
    // Also set in Blueprint break states for cross-compatibility
    UGWBLoopUtilsBlueprintLibrary::BlueprintBreakStates.Add(HandleID, true);
}

bool FBudgetedLoopHandle::ShouldBreak() const
{
    // Check both C++ flag and Blueprint break states
    return bShouldBreak || UGWBLoopUtilsBlueprintLibrary::BlueprintBreakStates.FindRef(HandleID);
}

void FBudgetedLoopHandle::Reset()
{
    bShouldBreak = false;
    // Clean up Blueprint break state
    UGWBLoopUtilsBlueprintLibrary::BlueprintBreakStates.Remove(HandleID);
}

void UGWBLoopUtilsBlueprintLibrary::BudgetedForLoopBlueprint(
    const UObject* WorldContextObject,
    float FrameBudget,
    int32 MaxWorkCount,
    int32 ArrayCount,
    const FGWBBudgetedLoopWorkDelegate& WorkDelegate,
    const FString& CallSiteId)
{
    // Handle edge cases
    if (!WorldContextObject || ArrayCount <= 0 || FrameBudget <= 0.0f)
    {
        return;
    }
    
    // Generate a unique ID if none provided
    FName UniqueId = NAME_None;
    if (CallSiteId.IsEmpty())
    {
        // Generate a consistent hash-based ID for Blueprint calls using hybrid approach
        // Combine object context with call stack for unique but consistent identification
        FString CallContext;
        
        if (const UBlueprintGeneratedClass* BPClass = Cast<UBlueprintGeneratedClass>(WorldContextObject->GetClass()))
        {
            // Blueprint context: use class name + object pointer for consistency across calls
            CallContext = FString::Printf(TEXT("%s_%p"), *BPClass->GetName(), WorldContextObject);
        }
        else
        {
            // Non-Blueprint context: use regular class name + object pointer
            CallContext = FString::Printf(TEXT("%s_%p"), *WorldContextObject->GetClass()->GetName(), WorldContextObject);
        }
        
        // Use object address + class for call site differentiation  
        // This provides reasonable uniqueness per object instance while being simple and fast
        
        // Create final unique ID from hash
        UniqueId = *FString::Printf(TEXT("BudgetedLoop_BP_%u"), GetTypeHash(CallContext));
    }
    else
    {
        UniqueId = *CallSiteId;
    }
    
    // Create the time-sliced reset scope
    FGWBTimeSlicedScope TimeSlicer(WorldContextObject, UniqueId, FrameBudget, MaxWorkCount);
    
    // Create loop handle for break functionality
    FBudgetedLoopHandle LoopHandle;
    LoopHandle.Reset(); // Ensure clean state
    
    // Iterate through the array count
    for (int32 Index = 0; Index < ArrayCount; ++Index)
    {
        FGWBTimeSlicedLoopScope TimeSlicedWork = TimeSlicer.StartLoopScope();
        // Check if we're over budget OR user requested break
        if (TimeSlicedWork.IsOverBudget() || LoopHandle.ShouldBreak())
        {
            break;
        }
        
        // Execute the work delegate if bound, passing the loop handle
        if (WorkDelegate.IsBound())
        {
            WorkDelegate.ExecuteIfBound(LoopHandle);
        }
    }
}

void UGWBLoopUtilsBlueprintLibrary::BreakBudgetedLoop(const FBudgetedLoopHandle& LoopHandle)
{
    // Set break state in the static map using the handle's unique ID
    BlueprintBreakStates.Add(LoopHandle.GetHandleID(), true);
}

bool UGWBLoopUtilsBlueprintLibrary::IsBudgetedLoopBroken(const FBudgetedLoopHandle& LoopHandle)
{
    // Check if this handle has been broken via Blueprint or C++
    return LoopHandle.ShouldBreak();
}