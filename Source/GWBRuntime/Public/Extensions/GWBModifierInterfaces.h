#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GWBModifierInterfaces.generated.h"

UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UGWBBudgetModifierInterface : public UInterface
{
    GENERATED_BODY()
};

class GWBRUNTIME_API IGWBBudgetModifierInterface
{
    GENERATED_BODY()

public:
    // Modify global frame budget
    virtual void ModifyFrameBudget(double& Budget, const FName& GroupId) = 0;
    
    // Modify group-specific budgets
    virtual void ModifyGroupBudget(const FName& GroupId, double& TimeBudget, int32& UnitCountBudget) = 0;
};

UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UGWBPriorityModifierInterface : public UInterface
{
    GENERATED_BODY()
};

class GWBRUNTIME_API IGWBPriorityModifierInterface
{
    GENERATED_BODY()

public:
    // Modify work group priority
    virtual void ModifyGroupPriority(const FName& GroupId, int32& Priority) = 0;
    
    // Modify work unit priority within a group
    virtual void ModifyWorkUnitPriority(const FName& GroupId, int32& Priority) = 0;
};

UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UGWBDeferredHandlerInterface : public UInterface
{
    GENERATED_BODY()
};

class GWBRUNTIME_API IGWBDeferredHandlerInterface
{
    GENERATED_BODY()

public:
    // Handle a deferred work group
    virtual void HandleDeferredWorkGroup(const FName& GroupId) = 0;
    
    // Handle a deferred work unit
    virtual void HandleDeferredWorkUnit(const FName& GroupId) = 0;
};