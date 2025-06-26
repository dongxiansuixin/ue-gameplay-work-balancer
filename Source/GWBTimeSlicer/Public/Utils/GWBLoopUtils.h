#pragma once

#include "CoreMinimal.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DataTypes/GWBTimeSlicedLoopScope.h"

// C++20 std::source_location support
#if __cplusplus >= 202002L && __has_include(<source_location>)
    #include <source_location>
    #define GWB_HAS_SOURCE_LOCATION 1
#else
    #define GWB_HAS_SOURCE_LOCATION 0
#endif

#include "GWBLoopUtils.generated.h"

/**
 * Handle passed to budgeted loop work functions that allows early termination of the loop.
 * Users can call Break() from within their work lambda/delegate to exit the loop immediately.
 */
USTRUCT(BlueprintType)
struct GWBTIMESLICER_API FBudgetedLoopHandle
{
    GENERATED_BODY()

public:
    FBudgetedLoopHandle()
    {
        // Generate unique ID for this handle instance
        HandleID = FMath::Rand32() ^ FPlatformTime::Cycles();
    }

    /**
     * Call this function from within your work lambda/delegate to break out of the loop early.
     * The loop will terminate after the current iteration completes.
     * NOTE: For Blueprint usage, use the "Break Budgeted Loop" function instead.
     */
    void Break();

    /**
     * Check if the loop should break (used internally by the loop implementation).
     * @return true if Break() has been called, false otherwise
     */
    bool ShouldBreak() const;

    /**
     * Reset the break flag (used internally by the loop implementation).
     */
    void Reset();

    /**
     * Get the unique ID for this handle (used for Blueprint break state tracking)
     */
    uint32 GetHandleID() const { return HandleID; }

private:
    /** Unique identifier for this handle instance */
    UPROPERTY()
    uint32 HandleID = 0;

    /** Flag indicating whether the loop should break early (C++ only) */
    bool bShouldBreak = false;
};

DECLARE_DYNAMIC_DELEGATE_OneParam(FGWBBudgetedLoopWorkDelegate, FBudgetedLoopHandle&, LoopHandle);

namespace GWBLoopUtils
{
#if GWB_HAS_SOURCE_LOCATION
    /**
     * BUDGETED FOR LOOP function for TArray containers (C++20 version with std::source_location)
     * Automatically generates unique call site identifiers using std::source_location.
     * 
     * Distributes expensive loop processing across multiple frames using FGWBTimeSlicedLoopScope
     * to maintain frame rate targets by respecting time and work unit count budgets.
     * Users can call Break() on the provided handle to exit the loop early.
     * 
     * @param WorldContext - UObject providing world context (usually 'this' from calling object)
     * @param FrameBudget - Time budget in seconds for this frame (e.g., 0.1f for 100ms)
     * @param MaxWorkCount - Maximum number of work units allowed per frame
     * @param Array - The TArray to iterate through
     * @param DoWork - Function/lambda that receives a break handle pointer for each iteration
     * @param Location - Source location (automatically captured at call site)
     * 
     * EXAMPLE:
     * ```cpp
     * // Automatic call site identification with C++20 and break capability
     * GWBLoopUtils::BudgetedForLoop(this, 0.1f, 10, MyArray, [&](FBudgetedLoopHandle& Handle) {
     *     // Process current array element
     *     DoSomeExpensiveProcessing();
     *     
     *     if (SomeCondition)
     *     {
     *         Handle.Break(); // Exit loop early
     *         return;
     *     }
     * });
     * ```
     */
    template<typename T>
    static void BudgetedForLoop(const UObject* WorldContext, float FrameBudget, uint32 MaxWorkCount, 
                               const TArray<T>& Array, TFunction<void(FBudgetedLoopHandle&)> DoWork, 
                               const std::source_location& Location = std::source_location::current())
    {
        // Handle edge cases
        if (!WorldContext || Array.Num() == 0 || FrameBudget <= 0.0f)
        {
            return;
        }
        
        // Generate unique ID from source location (hash file name, line, and function)
        FString LocationString = FString::Printf(TEXT("%s:%u:%s"), 
            ANSI_TO_TCHAR(Location.file_name()), 
            Location.line(), 
            ANSI_TO_TCHAR(Location.function_name()));
        
        // Hash to prevent file path leakage and create clean FName
        uint32 LocationHash = GetTypeHash(LocationString);
        FName UniqueId = *FString::Printf(TEXT("BudgetedLoop_%u"), LocationHash);
        
        // Create the time-sliced loop scope
        FGWBTimeSlicedLoopScope TimeSlicedWork(WorldContext, UniqueId, FrameBudget, MaxWorkCount);
        
        // Create loop handle for break functionality
        FBudgetedLoopHandle LoopHandle;
        LoopHandle.Reset(); // Ensure clean state
        
        // Iterate through the array
        for (int32 Index = 0; Index < Array.Num(); ++Index)
        {
            // Check if we're over budget OR user requested break
            if (TimeSlicedWork.IsOverBudget() || LoopHandle.ShouldBreak())
            {
                break;
            }
            
            // Execute the work function with handle
            DoWork(LoopHandle);
        }
    }
#else
    /**
     * BUDGETED FOR LOOP function for TArray containers (fallback version for pre-C++20)
     * Uses manual CallSiteId parameter for unique identification.
     * 
     * Distributes expensive loop processing across multiple frames using FGWBTimeSlicedLoopScope
     * to maintain frame rate targets by respecting time and work unit count budgets.
     * Users can call Break() on the provided handle to exit the loop early.
     * 
     * @param WorldContext - UObject providing world context (usually 'this' from calling object)
     * @param FrameBudget - Time budget in seconds for this frame (e.g., 0.1f for 100ms)
     * @param MaxWorkCount - Maximum number of work units allowed per frame
     * @param Array - The TArray to iterate through
     * @param DoWork - Function/lambda that receives a break handle pointer for each iteration
     * @param CallSiteId - Unique identifier for this call site (use macro for auto-generation)
     * 
     * EXAMPLE:
     * ```cpp
     * // Use the macro for automatic unique ID generation and break capability
     * BUDGETED_FOR_LOOP(this, 0.1f, 10, MyArray, [&](FBudgetedLoopHandle& Handle) {
     *     // Process current array element
     *     DoSomeExpensiveProcessing();
     *     
     *     if (SomeCondition)
     *     {
     *         Handle.Break(); // Exit loop early
     *         return;
     *     }
     * });
     * 
     * // Or provide manual ID
     * GWBLoopUtils::BudgetedForLoop(this, 0.1f, 10, MyArray, [&](FBudgetedLoopHandle& Handle) {
     *     // Process current array element with break capability
     * }, TEXT("MyCustomLoop"));
     * ```
     */
    template<typename T>
    static void BudgetedForLoop(const UObject* WorldContext, float FrameBudget, uint32 MaxWorkCount, 
                               const TArray<T>& Array, TFunction<void(FBudgetedLoopHandle&)> DoWork, const FName& CallSiteId = NAME_None)
    {
        // Handle edge cases
        if (!WorldContext || Array.Num() == 0 || FrameBudget <= 0.0f)
        {
            return;
        }
        
        // Generate a unique ID if none provided
        FName UniqueId = CallSiteId;
        if (UniqueId == NAME_None)
        {
            UniqueId = FName(TEXT("BudgetedForLoop_Default"));
        }
        
        // Create the time-sliced loop scope
        FGWBTimeSlicedLoopScope TimeSlicedWork(WorldContext, UniqueId, FrameBudget, MaxWorkCount);
        
        // Create loop handle for break functionality
        FBudgetedLoopHandle LoopHandle;
        LoopHandle.Reset(); // Ensure clean state
        
        // Iterate through the array
        for (int32 Index = 0; Index < Array.Num(); ++Index)
        {
            // Check if we're over budget OR user requested break
            if (TimeSlicedWork.IsOverBudget() || LoopHandle.ShouldBreak())
            {
                break;
            }
            
            // Execute the work function with handle
            DoWork(LoopHandle);
        }
    }
#endif
}

/**
 * Blueprint Function Library for GWB Loop Utilities
 * Provides Blueprint-accessible versions of budgeted loop functionality
 */
UCLASS()
class GWBTIMESLICER_API UGWBLoopUtilsBlueprintLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

    friend FBudgetedLoopHandle;

public:
    /**
     * Blueprint-callable budgeted for loop that distributes expensive processing across multiple frames.
     * This function will execute the work delegate repeatedly until either the frame time budget, 
     * maximum work count is exceeded, or the user calls Break() on the loop handle.
     * 
     * @param WorldContextObject - Object providing world context (connect to Self pin)
     * @param FrameBudget - Time budget in seconds for this frame (e.g., 0.1 for 100ms)
     * @param MaxWorkCount - Maximum number of work units allowed per frame
     * @param ArrayCount - Number of elements to process (mimics array length)
     * @param WorkDelegate - Blueprint event/function to call for each work unit (receives loop handle)
     * @param CallSiteId - Optional unique identifier for this loop (leave empty for auto-generation)
     * 
     * USAGE IN BLUEPRINTS:
     * 1. Call this function on Tick or when you want to process work
     * 2. Connect your expensive processing logic to the WorkDelegate
     * 3. Use the LoopHandle parameter with "Break Budgeted Loop" node to exit early
     * 4. The function will automatically break when budget is exceeded or Break() is called
     * 5. Call again next frame to continue processing
     */
    UFUNCTION(BlueprintCallable, Category = "GWB|Loop Utils", 
              meta = (DisplayName = "Budgeted For Loop", 
                      ToolTip = "Distributes expensive loop processing across frames to maintain performance. Use 'Break Budgeted Loop' node with LoopHandle to exit early.",
                      CallInEditor = "false"))
    static void BudgetedForLoopBlueprint(
        const UObject* WorldContextObject,
        float FrameBudget,
        int32 MaxWorkCount,
        int32 ArrayCount,
        const FGWBBudgetedLoopWorkDelegate& WorkDelegate,
        const FString& CallSiteId = TEXT("")
    );

    /**
     * Call this to break a budgeted loop early from Blueprint.
     * This function works with the loop handle passed to your work delegate.
     * 
     * @param LoopHandle - The handle passed to your work delegate
     */
    UFUNCTION(BlueprintCallable, Category = "GWB|Loop Utils", 
              meta = (DisplayName = "Break Budgeted Loop", 
                      ToolTip = "Break out of a budgeted loop early. Use with the handle from your work delegate."))
    static void BreakBudgetedLoop(const FBudgetedLoopHandle& LoopHandle);
    
    /**
     * Check if a loop handle has been broken (for Blueprint debugging).
     * 
     * @param LoopHandle - The handle to check
     * @return true if the loop has been broken, false otherwise
     */
    UFUNCTION(BlueprintPure, Category = "GWB|Loop Utils",
              meta = (DisplayName = "Is Budgeted Loop Broken"))
    static bool IsBudgetedLoopBroken(const FBudgetedLoopHandle& LoopHandle);

protected:
    /** Static map to track break states by handle ID for Blueprint usage */
    static TMap<uint32, bool> BlueprintBreakStates;
};

#if GWB_HAS_SOURCE_LOCATION
/**
 * Macro wrapper for BudgetedForLoop with C++20 std::source_location support.
 * Automatically captures call site information for perfect unique identification.
 * 
 * @param WorldContext - UObject providing world context (usually 'this' from calling object)
 * @param FrameBudget - Time budget in seconds for this frame (e.g., 0.1f for 100ms)
 * @param MaxWorkCount - Maximum number of work units allowed per frame
 * @param Array - The TArray to iterate through
 * @param DoWork - Function/lambda that receives a break handle for each iteration
 * 
 * USAGE:
 * ```cpp
 * BUDGETED_FOR_LOOP(this, 0.1f, 10, MyArray, [&](FBudgetedLoopHandle& Handle) {
 *     // Your expensive processing code here
 *     if (SomeCondition)
 *     {
 *         Handle.Break(); // Exit loop early
 *         return;
 *     }
 * });
 * ```
 */
#define BUDGETED_FOR_LOOP(WorldContext, FrameBudget, MaxWorkCount, Array, DoWork) \
    GWBLoopUtils::BudgetedForLoop(WorldContext, FrameBudget, MaxWorkCount, Array, DoWork)

#else
/**
 * Macro wrapper for BudgetedForLoop (fallback for pre-C++20) that generates unique call site identifiers
 * using a hash of __FILE__ and __LINE__ to ensure each call site gets its own time slicer instance
 * while preventing file path leakage at runtime.
 * 
 * @param WorldContext - UObject providing world context (usually 'this' from calling object)
 * @param FrameBudget - Time budget in seconds for this frame (e.g., 0.1f for 100ms)
 * @param MaxWorkCount - Maximum number of work units allowed per frame
 * @param Array - The TArray to iterate through
 * @param DoWork - Function/lambda that receives a break handle for each iteration
 * 
 * USAGE:
 * ```cpp
 * BUDGETED_FOR_LOOP(this, 0.1f, 10, MyArray, [&](FBudgetedLoopHandle& Handle) {
 *     // Your expensive processing code here
 *     if (SomeCondition)
 *     {
 *         Handle.Break(); // Exit loop early
 *         return;
 *     }
 * });
 * ```
 */
#define BUDGETED_FOR_LOOP(WorldContext, FrameBudget, MaxWorkCount, Array, DoWork) \
    GWBLoopUtils::BudgetedForLoop(WorldContext, FrameBudget, MaxWorkCount, Array, DoWork, \
        *FString::Printf(TEXT("BudgetedLoop_%u"), GetTypeHash(FString::Printf(TEXT("%s:%d"), ANSI_TO_TCHAR(__FILE__), __LINE__))))

#endif