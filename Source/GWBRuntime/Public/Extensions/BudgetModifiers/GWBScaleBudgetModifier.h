#pragma once

#include "CoreMinimal.h"
#include "Extensions/GWBModifierInterfaces.h"
#include "GWBScaleBudgetModifier.generated.h"

UCLASS(BlueprintType)
class GWBRUNTIME_API UGWBScaleBudgetModifier : public UObject, public IGWBBudgetModifierInterface
{
    GENERATED_BODY()

public:
    // Scale factor for the global frame budget
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget Modifier")
    float GlobalBudgetScale = 1.0f;
    
    // Scale factors for specific group budgets (GroupId -> Scale)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget Modifier")
    TMap<FName, float> GroupBudgetScales;
    
    // Implementation of IGWBBudgetModifierInterface
    virtual void ModifyFrameBudget(double& Budget, const FName& GroupId) override;
    virtual void ModifyGroupBudget(const FName& GroupId, double& TimeBudget, int32& UnitCountBudget) override;
};