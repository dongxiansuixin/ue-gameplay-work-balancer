#pragma once

#include "CoreMinimal.h"
#include "K2Node.h"
#include "K2Node_CallFunction.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_GWBScheduleWork.generated.h"

/**
 * Independent K2 Blueprint node for scheduling work with wildcard context support.
 * Handles everything directly without async action proxies or custom thunks.
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
	
	// Node generation helpers
	UK2Node_CallFunction* CreateCallFunctionNode(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph, UFunction* Function) const;
	UK2Node_CustomEvent* CreateCompletionEvent(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph, const FEdGraphPinType& ContextType) const;

private:
	// Pin names
	static const FName ContextInputPinName;
	static const FName WorkGroupPinName;
	static const FName WorkOptionsPinName;
	static const FName OnCompletedPinName;
	static const FName OnAbortedPinName;
	static const FName ContextOutputPinName;
	static const FName DeltaTimePinName;
	static const FName WorkHandlePinName;

	// Helper methods
	UEdGraphPin* GetContextInputPin() const;
	UEdGraphPin* GetContextOutputPin() const;
	UEdGraphPin* GetExecPin() const;
	UEdGraphPin* GetCompletedPin() const;
	UEdGraphPin* GetAbortedPin() const;
	
	// Context type information
	bool HasWildcardContextPins() const;
	FEdGraphPinType GetConnectedContextType() const;
};