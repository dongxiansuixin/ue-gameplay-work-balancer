// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "GWBBudgetModifier.generated.h"

/**
 * 
 */
UCLASS()
class GWBRUNTIME_API UGWBBudgetModifier : public UObject
{
	GENERATED_BODY()

	void ModifyBudget(double& Budget);
	void ModifyGroupBudget(double& Budget, uint32& MaxUnitCount);
};
