#pragma once

// unreal api
#include "CoreMinimal.h"
#include "UObject/Object.h"

// gameplay work balancer
#include "Components/GWBScheduler.h"
#include "DataTypes/GWBWorkGroup.h"
#include "DataTypes/GWBWorkUnit.h"
#include "DataTypes/GWBWorkOptions.h"
#include "DataTypes/GWBWorkUnitHandle.h"
#include "Extensions/ModifierManager.h"

#include "GWBManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGWBOnBeforeDoWorkDelegate, float, DeltaTime);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGWBBlueprintWorkDelegate, float, DeltaTime, const FGWBWorkUnitHandle&, Handle);

/**
 * Manages the work loop, tracks budgets, and fires off the work delegates when work needs to be done.
 *
 * To use the manager to schedule some work:
 * - Make sure your CVars are configured. The most important ones are `gwb.budget.frame` and `gwb.enabled`.
 * - Grab the singleton you can grab from `UGWBSubsystem::GetManager`
 * - Use `ScheduleWork` to schedule a work unit. It returns a handle that has a delegate you can access via `Handle.GetWorkCallback()`
 * - Bind a function to the delegate and do your work in that function. Your bound function will be called if there is room in the budget.
 */
UCLASS(config = Game)
class GWBRUNTIME_API UGWBManager : public UObject
{
	GENERATED_BODY()

// START for unit tests
#if WITH_DEV_AUTOMATION_TESTS
	friend class FGWBManagerTests;
	friend class FGWBExtensionsTests;
#endif
// END for unit tests
	
	friend class UGWBSubsystem;
	
public:

	UGWBManager();
	
	void Initialize(UWorld* ForWorld);
	
	/**
	 * @param WorkGroupId the group the work should be scheduled for.
	 * @param WorkOptions special options for this unit of work.
	 * @returns A work handle that can be used to register a callback when work is ready to be done
	 * NOTE: call AbortGameWork or abort the returned promise to cancel the work.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer", meta=(GameplayTagFilter="GameWork", WorldContext="WorldContextObject"))
	static FGWBWorkUnitHandle ScheduleWork(const UObject* WorldContextObject, UPARAM(meta = (GetOptions = "GetValidGroupNames")) FName WorkGroupId = "Default", const FGWBWorkOptions& WorkOptions = FGWBWorkOptions());
	
	/**
	 * Aborting a work unit is, unfortunately, expensive as it uses a handle indexed by Id and loops through
	 * all the groups and their work units to find the one to abort.
	 * ...Should be improved in the future.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer", meta=(GameplayTagFilter="GameWork", WorldContext="WorldContextObject"))
	static void AbortWorkUnit(const UObject* WorldContextObject, FGWBWorkUnitHandle WorkUnitHandle);

	/** Bind a Blueprint callback to a work handle. */
	UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer")
	static void BindBlueprintCallback(UPARAM(ref) FGWBWorkUnitHandle& Handle, const FGWBBlueprintWorkDelegate& OnDoWork);

	/** Delegate fired just before doing work for a frame, to allow external systems to just-in-time schedule work. */
	UPROPERTY()
	FGWBOnBeforeDoWorkDelegate OnBeforeDoWorkDelegate;

	/**
	 * Work group settings config that should be defined in the INI.
	 * It's totally fine to just have one work group, but since each group can have it's own budget
	 * you might find it helpful to break out things like VFX into a group and cleaning up dead enemies into another
	 * group.
	 */
	UPROPERTY(Config)
	TArray<FGWBWorkGroupDefinition> WorkGroupDefinitions;

protected:

	///
	/// <core-api>
	/// 
	void				Reset();
	FGWBWorkUnitHandle	ScheduleWork(const FName& WorkGroupId, const FGWBWorkOptions& WorkOptions);
	void				DoWork();
	void				DoWorkForGroup(FGWBWorkGroup& WorkGroup);
	void				DoWorkForUnit(const FGWBWorkUnit& WorkUnit) const;
	///
	/// </core-api>
	///

	///
	/// <extension-points>
	///
	void				ApplyBudgetModifiers(double& FrameBudget);
	void				ApplyGroupBudgetModifiers(FName GroupId, double& TimeBudget, int32& UnitCountBudget);
	void				OnWorkScheduled(FName GroupId);
	void				OnWorkDeferred(uint32 DeferredWorkUnitCount);
	void				OnWorkGroupDeferred(FName WorkGroupId);
	void				OnWorkUnitDeferred(FName WorkGroupId);
	///
	/// </extension-points>
	///
	
public:
	
	///
	/// <state>
	///
	bool				bIsDoingWork;
	uint32				TotalWorkCount;
	TSet<FGWBWorkGroup, FGWBWorkGroupSetKeyFuncs> WorkGroups;
	bool				bPendingReset;
	///
	/// </state>
	///
	///

	TArray<FName> GetValidGroupNames() const;

protected:
	
	TWeakObjectPtr<UGWBScheduler> Scheduler;
	FModifierManager ModifierManager; // Extension framework
};
