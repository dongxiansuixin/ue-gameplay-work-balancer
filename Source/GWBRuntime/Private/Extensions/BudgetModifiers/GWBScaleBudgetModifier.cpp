#include "Extensions/BudgetModifiers/GWBScaleBudgetModifier.h"

void UGWBScaleBudgetModifier::ModifyFrameBudget(double& Budget, const FName& GroupId)
{
    Budget *= GlobalBudgetScale;
}

void UGWBScaleBudgetModifier::ModifyGroupBudget(const FName& GroupId, double& TimeBudget, int32& UnitCountBudget)
{
    float* ScalePtr = GroupBudgetScales.Find(GroupId);
    if (ScalePtr)
    {
        TimeBudget *= *ScalePtr;
        // We only scale time budget, not unit count
    }
}