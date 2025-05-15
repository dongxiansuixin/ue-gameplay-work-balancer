#pragma once

// unreal api
#include "CoreMinimal.h"
#include "UObject/Object.h"

// gameplay work balancer
#include "Components/GWBScheduler.h"
#include "DataTypes/GWBWorkGroup.h"
#include "DataTypes/GWBWorkUnit.h"
#include "DataTypes/GWBWorkOptions.h"

#include "GWBManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGWBOnBeforeDoWorkDelegate, float, DeltaTime);

/**
 * 
 */
UCLASS(config = Game)
class GWBRUNTIME_API UGWBManager : public UObject
{
	GENERATED_BODY()

// START for unit tests
#if WITH_DEV_AUTOMATION_TESTS
	friend class FGWBManagerTests;
#endif
// END for unit tests
	
public:
	
	void Initialize();
	
	/**
	 * Schedules work in a group
	 * NOTE: call AbortGameWork or abort the returned promise to cancel the work.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer", meta=(GameplayTagFilter="GameWork"))
	static FGWBWorkUnitHandle ScheduleWork(const UObject* WorldContextObject, FName WorkGroupId, const FGWBWorkOptions& WorkOptions);
	
	UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer", meta=(GameplayTagFilter="GameWork"))
	static void AbortWorkUnit(const UObject* WorldContextObject, FGWBWorkUnitHandle WorkUnit);

	/** Delegate fired just before doing work for a frame, to allow external systems to just-in-time schedule work. */
	UPROPERTY()
	FGWBOnBeforeDoWorkDelegate OnBeforeDoWorkDelegate;

	UPROPERTY(Config)
	TArray<FGWBWorkGroupDefinition> WorkGroupDefinitions;

protected:

	void				Reset();
	FGWBWorkUnitHandle	ScheduleWork(const FName& WorkGroupId, const FGWBWorkOptions& WorkOptions);
	void				DoWork();
	void				DoWorkForGroup(FGWBWorkGroup& WorkGroup);
	void				DoWorkForUnit(const FGWBWorkUnit& WorkUnit) const;

	/// <extension-points>
	void				ApplyBudgetModifiers(double& FrameBudget);
	void				ApplyGroupBudgetModifiers(FName GroupId, double& TimeBudget, int32& UnitCountBudget);
	void				OnWorkGroupDeferred(FName WorkGroupId);
	void				OnWorkUnitDeferred(FName WorkGroupId);
	/// </extension-points>
	
public:
	// Register a budget modifier
	UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer|Extensions")
	void RegisterBudgetModifier(UObject* BudgetModifier);
	
	// Register a priority modifier
	UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer|Extensions")
	void RegisterPriorityModifier(UObject* PriorityModifier);
	
	// Register a deferred handler
	UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer|Extensions")
	void RegisterDeferredHandler(UObject* DeferredHandler);
	
	// Unregister modifiers
	UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer|Extensions")
	void UnregisterModifier(UObject* Modifier);
	
	/// <state>
	bool				bIsDoingWork;
	uint32				TotalWorkCount;
	TSet<FGWBWorkGroup, FGWBWorkGroupSetKeyFuncs> WorkGroups;
	bool				bPendingReset;
	double              EscalationScalar;      // Current escalation multiplier
	double              LastEscalationUpdate;  // Last time the escalation was updated
	uint32              DeferredWorkCount;     // Count of work units deferred in current frame

	// Extension framework
	TArray<TScriptInterface<IGWBBudgetModifierInterface>> BudgetModifiers;
	TArray<TScriptInterface<IGWBPriorityModifierInterface>> PriorityModifiers;
	TArray<TScriptInterface<IGWBDeferredHandlerInterface>> DeferredHandlers;
    /// </state>

protected:
	
	UPROPERTY() TObjectPtr<UGWBScheduler>	Scheduler;
};
