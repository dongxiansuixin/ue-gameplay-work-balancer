#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_GWBScheduleWork.generated.h"

/**
 * Independent K2 Blueprint node for scheduling work with wildcard context support.
 */
UCLASS()
class GWBEDITOR_API UK2Node_GWBScheduleWork : public UK2Node
{
	GENERATED_BODY()

public:
	UK2Node_GWBScheduleWork();

	// UK2Node interface
	virtual FText GetTooltipText() const override;
	virtual FText GetNodeTitle(ENodeTitleType::Type TitleType) const override;
	virtual FText GetMenuCategory() const override;
	virtual void GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const override;
	virtual FName GetCornerIcon() const override;
	virtual FSlateIcon GetIconAndTint(FLinearColor& OutColor) const override;
	virtual bool IsCompatibleWithGraph(const UEdGraph* TargetGraph) const override;

	// UK2Node interface - core functionality
	virtual void AllocateDefaultPins() override;
	virtual void PostReconstructNode() override;
	virtual void PinConnectionListChanged(UEdGraphPin* Pin) override;
	virtual void ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph) override;

	// Pin validation and connection handling
	virtual bool IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const override;

protected:
	// Wildcard pin management
	void PropagateWildcardPinTypes();
	void ResetWildcardPinTypes();

private:
	// Pin names
	static const FName ContextInputPinName;
	static const FName WorkGroupPinName;
	static const FName WorkOptionsPinName;
	static const FName OnDoWorkPinName;
	static const FName OnAbortedPinName;
	static const FName ContextOutputPinName;
	static const FName DeltaTimePinName;
	static const FName WorkHandlePinName;
	
	// Static map to track which graphs already have our shared variable
	static TMap<TWeakObjectPtr<UEdGraph>, FName> GraphSharedVariables;

	// Helper methods
	UEdGraphPin* GetContextInputPin() const;
	UEdGraphPin* GetContextOutputPin() const;
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetDoWorkPin() const;
	UEdGraphPin* GetAbortedPin() const;
	
	// Context type information
	bool HasWildcardContextPins() const;
	FEdGraphPinType GetConnectedContextType() const;
};