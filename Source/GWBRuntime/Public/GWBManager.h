#pragma once

// unreal api
#include "CoreMinimal.h"
#include "UObject/Object.h"

// gameplay work balancer
#include "Components/GWBScheduler.h"
#include "DataTypes/GWBWorkGroup.h"
#include "DataTypes/GWBWorkUnit.h"
#include "DataTypes/GWBWorkOptions.h"
#include "Extensions/ModifierManager.h"

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
	 * @param WorkGroupId the group the work should be scheduled for.
	 * @param WorkOptions special options for this unit of work.
	 * @returns A work handle that can be used to register a callback when work is ready to be done
	 * NOTE: call AbortGameWork or abort the returned promise to cancel the work.
	 */
	UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer", meta=(GameplayTagFilter="GameWork"))
	static FGWBWorkUnitHandle ScheduleWork(const UObject* WorldContextObject, FName WorkGroupId, const FGWBWorkOptions& WorkOptions);
	
	UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer", meta=(GameplayTagFilter="GameWork"))
	static void AbortWorkUnit(const UObject* WorldContextObject, FGWBWorkUnitHandle WorkUnit);

	/** Delegate fired just before doing work for a frame, to allow external systems to just-in-time schedule work. */
	UPROPERTY()
	FGWBOnBeforeDoWorkDelegate OnBeforeDoWorkDelegate;

	/** Work Group settings that can be defined in the INI */
	UPROPERTY(Config)
	TArray<FGWBWorkGroupDefinition> WorkGroupDefinitions;

protected:

	/// <core-api>
	void				Reset();
	FGWBWorkUnitHandle	ScheduleWork(const FName& WorkGroupId, const FGWBWorkOptions& WorkOptions);
	void				DoWork();
	void				DoWorkForGroup(FGWBWorkGroup& WorkGroup);
	void				DoWorkForUnit(const FGWBWorkUnit& WorkUnit) const;
	/// </core-api>

	/// <extension-points>
	void				ApplyBudgetModifiers(double& FrameBudget);
	void				ApplyGroupBudgetModifiers(FName GroupId, double& TimeBudget, int32& UnitCountBudget);
	void				OnWorkScheduled(FName GroupId);
	void				OnWorkDeferred(uint32 DeferredWorkUnitCount);
	void				OnWorkGroupDeferred(FName WorkGroupId);
	void				OnWorkUnitDeferred(FName WorkGroupId);
	/// </extension-points>
	
public:
	
	/// <state>
	bool				bIsDoingWork;
	uint32				TotalWorkCount;
	TSet<FGWBWorkGroup, FGWBWorkGroupSetKeyFuncs> WorkGroups;
	bool				bPendingReset;
	/// </state>

protected:
	
	UPROPERTY() TObjectPtr<UGWBScheduler>	Scheduler;
	FModifierManager ModifierManager; // Extension framework
};
