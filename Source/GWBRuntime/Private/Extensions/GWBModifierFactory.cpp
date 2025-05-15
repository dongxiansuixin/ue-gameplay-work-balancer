#include "Extensions/GWBModifierFactory.h"
#include "GWBManager.h"

UGWBLambdaBudgetModifier* UGWBModifierFactory::CreateLambdaBudgetModifier(UObject* Outer)
{
    return NewObject<UGWBLambdaBudgetModifier>(Outer);
}

UGWBLambdaPriorityModifier* UGWBModifierFactory::CreateLambdaPriorityModifier(UObject* Outer)
{
    return NewObject<UGWBLambdaPriorityModifier>(Outer);
}

UGWBLambdaDeferredHandler* UGWBModifierFactory::CreateLambdaDeferredHandler(UObject* Outer)
{
    return NewObject<UGWBLambdaDeferredHandler>(Outer);
}

UGWBLambdaBudgetModifier* UGWBModifierFactory::CreateGroupTimeMultiplier(UGWBManager* Manager, UObject* Outer, const FName& GroupId, float Multiplier)
{
    if (!Manager || !Outer)
    {
        return nullptr;
    }
    
    UGWBLambdaBudgetModifier* Modifier = CreateLambdaBudgetModifier(Outer);
    
    // Set up the group budget modifier
    Modifier->SetGroupBudgetModifier([GroupId, Multiplier](const FName& CurrentGroupId, double& TimeBudget, int32& UnitCountBudget) {
        if (CurrentGroupId == GroupId)
        {
            TimeBudget *= Multiplier;
        }
    });
    
    // Register with the manager
    Manager->RegisterBudgetModifier(Modifier);
    
    return Modifier;
}