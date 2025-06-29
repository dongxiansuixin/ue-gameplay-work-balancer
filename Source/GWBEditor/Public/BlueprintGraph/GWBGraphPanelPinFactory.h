

#pragma once

#include "CoreMinimal.h"
#include "Widgets/DeclarativeSyntaxSupport.h"
#include "EdGraphUtilities.h"
#include "EdGraphSchema_K2.h"
#include "GWBManager.h"
#include "GWBSubsystem.h"
#include "SGraphPin.h"
#include "SGraphPinNameList.h"

class FGWBGraphPanelPinFactory: public FGraphPanelPinFactory
{
	virtual TSharedPtr<class SGraphPin> CreatePin(class UEdGraphPin* InPin) const override
	{
		if (InPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Name && InPin->PinType.PinSubCategory == "GameplayWorkCategory")
		{
			TArray<TSharedPtr<FName>> RowNames;
			const UGWBSubsystem* Subsystem = GEngine->GetEngineSubsystem<UGWBSubsystem>();
			const UGWBManager* GlobalManager = Subsystem->GetManager();
			for (FName GroupName : GlobalManager->GetValidGroupNames())
			{
				RowNames.Add(MakeShared<FName>(GroupName));
			}
			RowNames.Add(MakeShareable(new FName("Default - Test")));
			return SNew(SGraphPinNameList, InPin, RowNames); // see also SGameplayTagGraphPin
		}
		return NULL;
	}
};
