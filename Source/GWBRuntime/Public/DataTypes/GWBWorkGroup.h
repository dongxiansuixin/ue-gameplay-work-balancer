#pragma once
#include "GWBWorkUnit.h"

#include "GWBWorkGroup.generated.h"

USTRUCT(BlueprintType)
struct GWBRUNTIME_API FGWBWorkGroupDefinition
{
	GENERATED_BODY()

	FGWBWorkGroupDefinition()
			: Id(NAME_None)
			, Priority(0)
			, bMutableWhileRunning(false)
			, MaxFrameBudget(0.0f)
			, MaxWorkUnitsPerFrame(0)
			, bCanSkipFrame(false)
			, bSkipUnlessFirstInFrame(false)
			, MaxNumSkippedFrames(0)
			, bAlwaysSkipUntilMax(false)
			, SkipPriorityDelta(0)
	{
	}

	/** Category identifier. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default", meta=(Categories="GameWork"))
	FName Id;

	/** Category priority (low to high). */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default")
	int32 Priority;

	/** Can new units of work be added and executed while this category is already running. Allows for recursion. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default")
	bool bMutableWhileRunning;

	static const FGWBWorkGroupDefinition EmptyDef;

    /// Time Slice Strategy Def

	/** How much time in seconds this category may spend doing work in a frame (i.e. 0.001 = 1ms). When not above 0, this setting is ignored and work occupies as much time as needed and is available in the frame. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default")
	float MaxFrameBudget;

	/** How many units of work this category is allowed to process in a frame. When not above 0, this setting is ignored and work occupies as much time as needed and is available in the frame. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default")
	int32 MaxWorkUnitsPerFrame;

	/** Whether this category is allowed to do no work in a frame, i.e. if higher priority categories use up all available work time. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default")
	bool bCanSkipFrame;

	/** Whether this category should do no work in a frame unless it is doing work first. This is useful for categories where the work to be done is known to be expensive and may exceed the overall budget. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default", meta = (EditCondition = "bCanSkipFrame"))
	bool bSkipUnlessFirstInFrame;

	/** Maximum number of frames allowed to skip before work for this category must be done (i.e. 0.001 = 1ms). When <= 0, this setting is ignored and work may be deferred. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default", meta = (EditCondition = "bCanSkipFrame"))
	int32 MaxNumSkippedFrames;

	/** Whether this category should always skip up to the max number of skipped frames. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default", meta = (EditCondition = "bCanSkipFrame"))
	bool bAlwaysSkipUntilMax;

	/** Amount to change priority by when category is skipped in a frame. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Default", meta = (EditCondition = "bCanSkipFrame"))
	int32 SkipPriorityDelta;
};

USTRUCT()
struct GWBRUNTIME_API FGWBWorkGroup
{
	GENERATED_BODY()

	FGWBWorkGroup()
			: Def()
			, NumWorkUnitsWithMaxDelay(0)
			, PriorityOffset(0)
			, NumSkippedFrames(0)
			, AverageUnitTime(0.0)
	{
	}
    
	FGWBWorkGroup(const FGWBWorkGroupDefinition& InDef)
		: Def(InDef)
		, NumWorkUnitsWithMaxDelay(0)
		, PriorityOffset(0)
		, NumSkippedFrames(0)
		, AverageUnitTime(0.0)
	{
	}

	/** Copy of category definition for quick access. */
	UPROPERTY()
	FGWBWorkGroupDefinition Def;

	/// <runtime_state>
	UPROPERTY() TArray<FGWBWorkUnit> WorkUnitsQueue; /** Work units in priority order. */
	UPROPERTY() int32 NumWorkUnitsWithMaxDelay;
	UPROPERTY() int32 PriorityOffset;
	UPROPERTY() int32 NumSkippedFrames;
	UPROPERTY() double AverageUnitTime;
    /// </runtime_state>

	FORCEINLINE int32 GetPriority() const { return Def.Priority + PriorityOffset; }
};

struct FGWBWorkGroupSetKeyFuncs : BaseKeyFuncs<FGWBWorkGroup, FName, false>
{
	static FORCEINLINE KeyInitType GetSetKey(ElementInitType const& Element) { return Element.Def.Id; }
	static FORCEINLINE uint32 GetKeyHash(KeyInitType const& Key) { return GetTypeHash(Key); }
	static FORCEINLINE bool Matches(KeyInitType const& A, KeyInitType const& B) { return (A == B); }
};