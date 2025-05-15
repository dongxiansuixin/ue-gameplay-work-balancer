#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Extensions/GWBModifierInterfaces.h"
#include "GWBModifierFactory.generated.h"

// Forward declarations
class UGWBManager;

UCLASS(BlueprintType)
class GWBRUNTIME_API UGWBLambdaBudgetModifier : public UObject, public IGWBBudgetModifierInterface
{
    GENERATED_BODY()

public:
    typedef TFunction<void(double&, const FName&)> FFrameBudgetModifyFunction;
    typedef TFunction<void(const FName&, double&, int32&)> FGroupBudgetModifyFunction;
    
    void SetFrameBudgetModifier(const FFrameBudgetModifyFunction& InFunction)
    {
        FrameBudgetModifier = InFunction;
    }
    
    void SetGroupBudgetModifier(const FGroupBudgetModifyFunction& InFunction)
    {
        GroupBudgetModifier = InFunction;
    }
    
    // Implementation of IGWBBudgetModifierInterface
    virtual void ModifyFrameBudget(double& Budget, const FName& GroupId) override
    {
        if (FrameBudgetModifier)
        {
            FrameBudgetModifier(Budget, GroupId);
        }
    }
    
    virtual void ModifyGroupBudget(const FName& GroupId, double& TimeBudget, int32& UnitCountBudget) override
    {
        if (GroupBudgetModifier)
        {
            GroupBudgetModifier(GroupId, TimeBudget, UnitCountBudget);
        }
    }
    
private:
    FFrameBudgetModifyFunction FrameBudgetModifier;
    FGroupBudgetModifyFunction GroupBudgetModifier;
};

UCLASS(BlueprintType)
class GWBRUNTIME_API UGWBLambdaPriorityModifier : public UObject, public IGWBPriorityModifierInterface
{
    GENERATED_BODY()

public:
    typedef TFunction<void(const FName&, int32&)> FPriorityModifyFunction;
    
    void SetGroupPriorityModifier(const FPriorityModifyFunction& InFunction)
    {
        GroupPriorityModifier = InFunction;
    }
    
    void SetWorkUnitPriorityModifier(const FPriorityModifyFunction& InFunction)
    {
        WorkUnitPriorityModifier = InFunction;
    }
    
    // Implementation of IGWBPriorityModifierInterface
    virtual void ModifyGroupPriority(const FName& GroupId, int32& Priority) override
    {
        if (GroupPriorityModifier)
        {
            GroupPriorityModifier(GroupId, Priority);
        }
    }
    
    virtual void ModifyWorkUnitPriority(const FName& GroupId, int32& Priority) override
    {
        if (WorkUnitPriorityModifier)
        {
            WorkUnitPriorityModifier(GroupId, Priority);
        }
    }
    
private:
    FPriorityModifyFunction GroupPriorityModifier;
    FPriorityModifyFunction WorkUnitPriorityModifier;
};

UCLASS(BlueprintType)
class GWBRUNTIME_API UGWBLambdaDeferredHandler : public UObject, public IGWBDeferredHandlerInterface
{
    GENERATED_BODY()

public:
    typedef TFunction<void(const FName&)> FDeferredHandlerFunction;
    
    void SetGroupDeferredHandler(const FDeferredHandlerFunction& InFunction)
    {
        GroupDeferredHandler = InFunction;
    }
    
    void SetWorkUnitDeferredHandler(const FDeferredHandlerFunction& InFunction)
    {
        WorkUnitDeferredHandler = InFunction;
    }
    
    // Implementation of IGWBDeferredHandlerInterface
    virtual void HandleDeferredWorkGroup(const FName& GroupId) override
    {
        if (GroupDeferredHandler)
        {
            GroupDeferredHandler(GroupId);
        }
    }
    
    virtual void HandleDeferredWorkUnit(const FName& GroupId) override
    {
        if (WorkUnitDeferredHandler)
        {
            WorkUnitDeferredHandler(GroupId);
        }
    }
    
private:
    FDeferredHandlerFunction GroupDeferredHandler;
    FDeferredHandlerFunction WorkUnitDeferredHandler;
};

// Factory class for creating modifiers
UCLASS()
class GWBRUNTIME_API UGWBModifierFactory : public UObject
{
    GENERATED_BODY()
    
public:
    // Create a lambda-based budget modifier
    UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer|Extensions")
    static UGWBLambdaBudgetModifier* CreateLambdaBudgetModifier(UObject* Outer);
    
    // Create a lambda-based priority modifier
    UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer|Extensions")
    static UGWBLambdaPriorityModifier* CreateLambdaPriorityModifier(UObject* Outer);
    
    // Create a lambda-based deferred handler
    UFUNCTION(BlueprintCallable, Category = "GameWorkBalancer|Extensions")
    static UGWBLambdaDeferredHandler* CreateLambdaDeferredHandler(UObject* Outer);
    
    // Helper to create and register a simple group time multiplier
    static UGWBLambdaBudgetModifier* CreateGroupTimeMultiplier(UGWBManager* Manager, UObject* Outer, const FName& GroupId, float Multiplier);
};