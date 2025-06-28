#include "BlueprintGraph/K2Node_GWBScheduleWork.h"
#include "GWBManager.h"
#include "DataTypes/GWBWorkOptions.h"
#include "DataTypes/GWBWorkUnitHandle.h"
#include "BlueprintActionDatabaseRegistrar.h"
#include "BlueprintNodeSpawner.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_AssignmentStatement.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_TemporaryVariable.h"
#include "K2Node_CreateDelegate.h"
#include "K2Node_Knot.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "KismetCompiler.h"
#include "GWBWildcardValueCache.h"
#include "K2Node_ExecutionSequence.h"
#include "Kismet/BlueprintMapLibrary.h"
#include "Kismet/KismetNodeHelperLibrary.h"

#define LOCTEXT_NAMESPACE "K2Node_GWBScheduleWorkDirect"

// Pin name constants
const FName UK2Node_GWBScheduleWork::ContextInputPinName(TEXT("Context"));
const FName UK2Node_GWBScheduleWork::WorkGroupPinName(TEXT("WorkGroup"));
const FName UK2Node_GWBScheduleWork::WorkOptionsPinName(TEXT("WorkOptions"));
const FName UK2Node_GWBScheduleWork::OnCompletedPinName(TEXT("OnCompleted"));
const FName UK2Node_GWBScheduleWork::OnAbortedPinName(TEXT("OnAborted"));
const FName UK2Node_GWBScheduleWork::ContextOutputPinName(TEXT("ContextOut"));
const FName UK2Node_GWBScheduleWork::DeltaTimePinName(TEXT("DeltaTime"));
const FName UK2Node_GWBScheduleWork::WorkHandlePinName(TEXT("WorkHandle"));

// Static map to track which graphs already have our shared variable
TMap<TWeakObjectPtr<UEdGraph>, FName> UK2Node_GWBScheduleWork::GraphSharedVariables;

UK2Node_GWBScheduleWork::UK2Node_GWBScheduleWork()
{
	// Set node properties
	bCanRenameNode = false;
}

FText UK2Node_GWBScheduleWork::GetTooltipText() const
{
	return LOCTEXT("NodeTooltip", "Schedules work to be executed when there is room in the frame budget. Context value is passed through and returned when work completes.");
}

FText UK2Node_GWBScheduleWork::GetNodeTitle(ENodeTitleType::Type TitleType) const
{
	return LOCTEXT("NodeTitle", "Schedule Work (Direct)");
}

FText UK2Node_GWBScheduleWork::GetMenuCategory() const
{
	return LOCTEXT("NodeCategory", "Game Work Balancer");
}

void UK2Node_GWBScheduleWork::GetMenuActions(FBlueprintActionDatabaseRegistrar& ActionRegistrar) const
{
	UClass* ActionKey = GetClass();
	if (ActionRegistrar.IsOpenForRegistration(ActionKey))
	{
		UBlueprintNodeSpawner* NodeSpawner = UBlueprintNodeSpawner::Create(GetClass());
		check(NodeSpawner != nullptr);
		ActionRegistrar.AddBlueprintAction(ActionKey, NodeSpawner);
	}
}

FName UK2Node_GWBScheduleWork::GetCornerIcon() const
{
	return TEXT("Graph.Latent.LatentIcon");
}

FSlateIcon UK2Node_GWBScheduleWork::GetIconAndTint(FLinearColor& OutColor) const
{
	OutColor = FLinearColor::White;
	return FSlateIcon("EditorStyle", "Kismet.AllClasses.FunctionIcon");
}

bool UK2Node_GWBScheduleWork::IsCompatibleWithGraph(const UEdGraph* TargetGraph) const
{
	// Only allow in function/event graphs, not in macro libraries
	EGraphType GraphType = TargetGraph->GetSchema()->GetGraphType(TargetGraph);
	return GraphType == GT_Function || GraphType == GT_Ubergraph;
}

void UK2Node_GWBScheduleWork::AllocateDefaultPins()
{
	// Input execution pin
	CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Execute);
	
	// Output execution pin
	CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, UEdGraphSchema_K2::PN_Then);

	// Context input pin (wildcard)
	UEdGraphPin* ContextInputPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Wildcard, ContextInputPinName);
	ContextInputPin->PinFriendlyName = LOCTEXT("ContextInputPinFriendlyName", "Context");

	// Work group input pin
	UEdGraphPin* WorkGroupPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Name, WorkGroupPinName);
	WorkGroupPin->PinFriendlyName = LOCTEXT("WorkGroupPinFriendlyName", "Work Group");
	WorkGroupPin->DefaultValue = TEXT("Default");
	WorkGroupPin->PinType.PinSubCategory = FName(TEXT("GameplayWorkCategory"));

	// Work options input pin
	UEdGraphPin* WorkOptionsPin = CreatePin(EGPD_Input, UEdGraphSchema_K2::PC_Struct, 
		FGWBWorkOptions::StaticStruct(), WorkOptionsPinName);
	WorkOptionsPin->PinFriendlyName = LOCTEXT("WorkOptionsPinFriendlyName", "Work Options");

	// Output execution pins
	UEdGraphPin* OnCompletedPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, OnCompletedPinName);
	OnCompletedPin->PinFriendlyName = LOCTEXT("OnCompletedPinFriendlyName", "On Completed");

	UEdGraphPin* OnAbortedPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Exec, OnAbortedPinName);
	OnAbortedPin->PinFriendlyName = LOCTEXT("OnAbortedPinFriendlyName", "On Aborted");

	// Output data pins
	UEdGraphPin* ContextOutputPin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Wildcard, ContextOutputPinName);
	ContextOutputPin->PinFriendlyName = LOCTEXT("ContextOutputPinFriendlyName", "Context");

	UEdGraphPin* DeltaTimePin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Real, UEdGraphSchema_K2::PC_Float, DeltaTimePinName);
	DeltaTimePin->PinFriendlyName = LOCTEXT("DeltaTimePinFriendlyName", "Delta Time");

	UEdGraphPin* WorkHandlePin = CreatePin(EGPD_Output, UEdGraphSchema_K2::PC_Struct, 
		FGWBWorkUnitHandle::StaticStruct(), WorkHandlePinName);
	WorkHandlePin->PinFriendlyName = LOCTEXT("WorkHandlePinFriendlyName", "Work Handle");

	Super::AllocateDefaultPins();
}

void UK2Node_GWBScheduleWork::PostReconstructNode()
{
	Super::PostReconstructNode();
	PropagateWildcardPinTypes();
}

void UK2Node_GWBScheduleWork::PinConnectionListChanged(UEdGraphPin* Pin)
{
	Super::PinConnectionListChanged(Pin);
	
	if (Pin && (Pin->PinName == ContextInputPinName || Pin->PinName == ContextOutputPinName))
	{
		PropagateWildcardPinTypes();
	}
}

void UK2Node_GWBScheduleWork::PropagateWildcardPinTypes()
{
	UEdGraphPin* ContextInputPin = GetContextInputPin();
	UEdGraphPin* ContextOutputPin = GetContextOutputPin();
	
	if (!ContextInputPin || !ContextOutputPin)
	{
		return;
	}
	
	bool bPinTypeChanged = false;
	
	// If context input is connected, propagate its type to output
	if (ContextInputPin->LinkedTo.Num() > 0)
	{
		UEdGraphPin* LinkedPin = ContextInputPin->LinkedTo[0];
		if (ContextOutputPin->PinType != LinkedPin->PinType)
		{
			ContextOutputPin->PinType = LinkedPin->PinType;
			bPinTypeChanged = true;
		}
		
		if (ContextInputPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
		{
			ContextInputPin->PinType = LinkedPin->PinType;
			bPinTypeChanged = true;
		}
	}
	// If context output is connected, propagate its type to input
	else if (ContextOutputPin->LinkedTo.Num() > 0)
	{
		UEdGraphPin* LinkedPin = ContextOutputPin->LinkedTo[0];
		if (ContextInputPin->PinType != LinkedPin->PinType)
		{
			ContextInputPin->PinType = LinkedPin->PinType;
			bPinTypeChanged = true;
		}
		
		if (ContextOutputPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
		{
			ContextOutputPin->PinType = LinkedPin->PinType;
			bPinTypeChanged = true;
		}
	}
	// If neither is connected, reset to wildcard
	else
	{
		ResetWildcardPinTypes();
	}
	
	if (bPinTypeChanged)
	{
		GetGraph()->NotifyGraphChanged();
	}
}

void UK2Node_GWBScheduleWork::ResetWildcardPinTypes()
{
	UEdGraphPin* ContextInputPin = GetContextInputPin();
	UEdGraphPin* ContextOutputPin = GetContextOutputPin();
	
	if (ContextInputPin && ContextInputPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
	{
		ContextInputPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
		ContextInputPin->PinType.PinSubCategory = NAME_None;
		ContextInputPin->PinType.PinSubCategoryObject = nullptr;
	}
	
	if (ContextOutputPin && ContextOutputPin->PinType.PinCategory != UEdGraphSchema_K2::PC_Wildcard)
	{
		ContextOutputPin->PinType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
		ContextOutputPin->PinType.PinSubCategory = NAME_None;
		ContextOutputPin->PinType.PinSubCategoryObject = nullptr;
	}
}

bool UK2Node_GWBScheduleWork::IsConnectionDisallowed(const UEdGraphPin* MyPin, const UEdGraphPin* OtherPin, FString& OutReason) const
{
	// Allow wildcard pins to connect to any compatible type
	if (MyPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard)
	{
		return false;
	}
	
	return Super::IsConnectionDisallowed(MyPin, OtherPin, OutReason);
}

// Helper methods for pin access
UEdGraphPin* UK2Node_GWBScheduleWork::GetContextInputPin() const
{
	return FindPin(ContextInputPinName, EGPD_Input);
}

UEdGraphPin* UK2Node_GWBScheduleWork::GetContextOutputPin() const
{
	return FindPin(ContextOutputPinName, EGPD_Output);
}

UEdGraphPin* UK2Node_GWBScheduleWork::GetExecPin() const
{
	return FindPin(UEdGraphSchema_K2::PN_Execute, EGPD_Input);
}

UEdGraphPin* UK2Node_GWBScheduleWork::GetCompletedPin() const
{
	return FindPin(OnCompletedPinName, EGPD_Output);
}

UEdGraphPin* UK2Node_GWBScheduleWork::GetAbortedPin() const
{
	return FindPin(OnAbortedPinName, EGPD_Output);
}

bool UK2Node_GWBScheduleWork::HasWildcardContextPins() const
{
	UEdGraphPin* ContextInputPin = GetContextInputPin();
	return ContextInputPin && ContextInputPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Wildcard;
}

FEdGraphPinType UK2Node_GWBScheduleWork::GetConnectedContextType() const
{
	UEdGraphPin* ContextInputPin = GetContextInputPin();
	if (ContextInputPin && ContextInputPin->LinkedTo.Num() > 0)
	{
		return ContextInputPin->LinkedTo[0]->PinType;
	}
	
	// Return wildcard type as default
	FEdGraphPinType WildcardType;
	WildcardType.PinCategory = UEdGraphSchema_K2::PC_Wildcard;
	return WildcardType;
}

FName UK2Node_GWBScheduleWork::GetOrCreateSharedVariable(FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph,
	FName ValueType)
{
	// Check if we already created the shared variable for this graph
	if (FName* ExistingVar = GraphSharedVariables.Find(SourceGraph))
	{
		return *ExistingVar;
	}
        
	// Create the shared variable for this graph
	const FName UniqueVarName = *FString::Printf(TEXT("SharedMap_%s"), *FGuid::NewGuid().ToString());
        
	// Create a Blueprint variable in the owning Blueprint
	if (UBlueprint* Blueprint = FBlueprintEditorUtils::FindBlueprintForGraph(SourceGraph))
	{
		// Create the variable property
		FEdGraphPinType MapPinType;
		MapPinType.PinCategory = ValueType;
		// MapPinType.PinSubCategoryObject = TBaseStructure<TMap<int32, UObject*>>::Get();
		MapPinType.ContainerType = EPinContainerType::Map;
		MapPinType.PinValueType.TerminalCategory = UEdGraphSchema_K2::PC_Int; // Key type
		MapPinType.PinValueType.TerminalSubCategory = ValueType; // Value type
            
		// Add variable to Blueprint
		FBlueprintEditorUtils::AddMemberVariable(Blueprint, UniqueVarName, MapPinType);
            
		// Mark Blueprint as modified
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		
		// Force reconstruction of all nodes to recognize the new variable
		FBlueprintEditorUtils::ReconstructAllNodes(Blueprint);
		
		// Refresh the Blueprint's variable list
		FBlueprintEditorUtils::RefreshAllNodes(Blueprint);
	}
        
	// Cache the variable name for this graph
	GraphSharedVariables.Add(SourceGraph, UniqueVarName);
        
	return UniqueVarName;
}

UE_DISABLE_OPTIMIZATION

void UK2Node_GWBScheduleWork::ExpandNode(class FKismetCompilerContext& CompilerContext, UEdGraph* SourceGraph)
{
	Super::ExpandNode(CompilerContext, SourceGraph);
	
	// INPUT pins
	UEdGraphPin* ExecInputPin = GetExecPin();
	UEdGraphPin* ThenOutPin = GetThenPin();
	UEdGraphPin* ContextInputPin = GetContextInputPin();
	UEdGraphPin* WorkGroupPin = FindPin(WorkGroupPinName, EGPD_Input);
	UEdGraphPin* WorkOptionsPin = FindPin(WorkOptionsPinName, EGPD_Input);
	// OUTPUT pins
	UEdGraphPin* OnCompletedPin = GetCompletedPin();
	UEdGraphPin* OnAbortedPin = GetAbortedPin();
	UEdGraphPin* ContextOutputPin = GetContextOutputPin();
	UEdGraphPin* DeltaTimePin = FindPin(DeltaTimePinName, EGPD_Output);
	UEdGraphPin* WorkHandlePin = FindPin(WorkHandlePinName, EGPD_Output);
	
	// Validate pins
	if (!ExecInputPin || !ContextInputPin || !WorkGroupPin || !WorkOptionsPin || 
		!OnCompletedPin || !OnAbortedPin || !ContextOutputPin || !DeltaTimePin || !WorkHandlePin)
	{
		CompilerContext.MessageLog.Error(TEXT("UK2Node_GWBScheduleWork: Missing required pins during expansion"));
		return;
	}
	
	// Get UGWBManager class once
	UClass* GWBManagerClass = UGWBManager::StaticClass();

	// 1. Setup context storage (if needed)
	FEdGraphPinType ContextType = GetConnectedContextType();
	bool bHasValidContext = ContextType.PinCategory != UEdGraphSchema_K2::PC_Wildcard;

	// connect the ExecIn to a knot we can use later to re-route based on if this node has a context var or not 
	UK2Node_Knot* ExecInRedirectorNode = CompilerContext.SpawnIntermediateNode<UK2Node_Knot>(this, SourceGraph);
	ExecInRedirectorNode->AllocateDefaultPins();
	CompilerContext.MovePinLinksToIntermediate(*ExecInputPin, *ExecInRedirectorNode->GetInputPin());
	UEdGraphPin* NextThenPin = ExecInRedirectorNode->GetOutputPin();
	
	// 2. Create ScheduleWork call
	UK2Node_CallFunction* ScheduleWorkNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	UFunction* ScheduleWorkFunction = GWBManagerClass->FindFunctionByName(TEXT("ScheduleWork"));
	ScheduleWorkNode->SetFromFunction(ScheduleWorkFunction);
	ScheduleWorkNode->AllocateDefaultPins();
	
	// Connect ScheduleWork inputs
	if (UEdGraphPin* WorldContextPin = ScheduleWorkNode->FindPinChecked(TEXT("WorldContextObject")))
	{
		ScheduleWorkNode->FindPinChecked(TEXT("WorldContextObject"))->DefaultValue = TEXT("None"); // Will be resolved at runtime
	}
	if (UEdGraphPin* WorkGroupIdPin = ScheduleWorkNode->FindPinChecked(TEXT("WorkGroupId")))
	{
		CompilerContext.MovePinLinksToIntermediate(*WorkGroupPin, *WorkGroupIdPin);
	}
	if (UEdGraphPin* WorkOptionsIdPin = ScheduleWorkNode->FindPinChecked(TEXT("WorkOptions")))
	{
		CompilerContext.MovePinLinksToIntermediate(*WorkOptionsPin, *WorkOptionsIdPin);
	}
	
	// 3. Create BindBlueprintCallback call
	UK2Node_CallFunction* BindCallbackNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
	UFunction* BindCallbackFunction = GWBManagerClass->FindFunctionByName(TEXT("BindBlueprintCallback"));
	BindCallbackNode->SetFromFunction(BindCallbackFunction);
	BindCallbackNode->AllocateDefaultPins();
	
	// 4. Create custom event for work completion callback
	UK2Node_CustomEvent* CompletionEvent = CompilerContext.SpawnIntermediateNode<UK2Node_CustomEvent>(this, SourceGraph);
	CompletionEvent->CustomFunctionName = FName(FString::Printf(TEXT("GWBWorkCompleted_%s"), *CompilerContext.GetGuid(this)));
	CompletionEvent->bCallInEditor = false;
	
	// Add DeltaTime parameter to the custom event
	FEdGraphPinType DeltaTimePinType;
	DeltaTimePinType.PinCategory = UEdGraphSchema_K2::PC_Real;
	DeltaTimePinType.PinSubCategory = UEdGraphSchema_K2::PC_Float;
	UEdGraphPin* EventDeltaTimePin = CompletionEvent->CreateUserDefinedPin(DeltaTimePinName, DeltaTimePinType, EGPD_Output);
	EventDeltaTimePin->PinFriendlyName = LOCTEXT("EventDeltaTimePinFriendlyName", "Delta Time");

	// Add WorkUnitHandle param to the custom event
	FEdGraphPinType WorkUnitHandlePinType;
	WorkUnitHandlePinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
	WorkUnitHandlePinType.bIsReference = true;
	WorkUnitHandlePinType.PinSubCategoryObject = TBaseStructure<FGWBWorkUnitHandle>::Get();
	CompletionEvent->CreateUserDefinedPin(WorkHandlePinName, WorkUnitHandlePinType, EGPD_Output);
	
	CompletionEvent->ReconstructNode();
	
	// 5. Create delegate for the custom event
	UK2Node_CreateDelegate* CreateDelegateNode = CompilerContext.SpawnIntermediateNode<UK2Node_CreateDelegate>(this, SourceGraph);
	CreateDelegateNode->SelectedFunctionName = CompletionEvent->CustomFunctionName;
	CreateDelegateNode->SelectedFunctionGuid = CompletionEvent->NodeGuid;
	CreateDelegateNode->AllocateDefaultPins();
	
	// 7. Wire execution flow: Exec -> [StoreContext] -> ScheduleWork -> BindCallback -> Then
	NextThenPin->MakeLinkTo(ScheduleWorkNode->GetExecPin());
	ScheduleWorkNode->GetThenPin()->MakeLinkTo(BindCallbackNode->GetExecPin());
	CompilerContext.MovePinLinksToIntermediate(*ThenOutPin, *(BindCallbackNode->GetThenPin()));
	
	// 8. Wire data connections
	// bind the handle coming out of the schedule work node to the BindCallback and this node's WorkHandle output
	UEdGraphPin* ScheduleWorkReturnPin = ScheduleWorkNode->GetReturnValuePin();
	UEdGraphPin* BindCallbackHandleInputPin = BindCallbackNode->FindPin(TEXT("Handle"));
	ScheduleWorkReturnPin->MakeLinkTo(BindCallbackHandleInputPin);
	CompilerContext.MovePinLinksToIntermediate(*WorkHandlePin, *ScheduleWorkReturnPin);
	
	// Connect delegate to BindCallback
	UEdGraphPin* OnDoWorkPin = BindCallbackNode->FindPin(TEXT("OnDoWork"));
	UEdGraphPin* DelegateOutputPin = CreateDelegateNode->GetDelegateOutPin();
	DelegateOutputPin->MakeLinkTo(OnDoWorkPin);
		
	// 9. Wire completion custom event to our output pins
	UEdGraphPin* EventThenPin = CompletionEvent->FindPin(UEdGraphSchema_K2::PN_Then, EGPD_Output);
	UEdGraphPin* EventDeltaTimePinOut = CompletionEvent->FindPin(DeltaTimePinName, EGPD_Output);
	UEdGraphPin* EventWorkHandlePinOut = CompletionEvent->FindPin(WorkHandlePinName, EGPD_Output);
	CompilerContext.MovePinLinksToIntermediate(*OnCompletedPin, *EventThenPin);
	CompilerContext.MovePinLinksToIntermediate(*DeltaTimePin, *EventDeltaTimePinOut);

	if (bHasValidContext)
	{
		// Get the ID of the work handle pin coming out of our schedule work node
		UK2Node_CallFunction* GetWorkUnitHandleIdNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		GetWorkUnitHandleIdNode->FunctionReference.SetExternalMember(FName("GetWorkUnitHandleId"), UGWBWildcardValueCache::StaticClass());
		GetWorkUnitHandleIdNode->bDefaultsToPureFunc = true;
		GetWorkUnitHandleIdNode->AllocateDefaultPins();
		UEdGraphPin* GetWorkUnitHandleIdInputPin = GetWorkUnitHandleIdNode->FindPin(TEXT("Handle"));
		UEdGraphPin* WorkUnitHandleReturnedIdPin = GetWorkUnitHandleIdNode->FindPin(TEXT("ReturnValue"));
		ScheduleWorkReturnPin->MakeLinkTo(GetWorkUnitHandleIdInputPin);
		
		const auto FunctionName_Assign = UGWBWildcardValueCache::GetAssignFunctionName(ContextType.PinCategory, ContextType.PinSubCategoryObject.Get());
		UK2Node_CallFunction* MapAddNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		MapAddNode->FunctionReference.SetExternalMember(FName(FunctionName_Assign), UGWBWildcardValueCache::StaticClass());
		MapAddNode->AllocateDefaultPins();
		UEdGraphPin* AddMapKeyExec = MapAddNode->GetExecPin();
		UEdGraphPin* AddMapKeyPin = MapAddNode->FindPin(TEXT("Key"));
		UEdGraphPin* AddMapValuePin = MapAddNode->FindPin(TEXT("Value"));
		UEdGraphPin* AddMapKeyThen = MapAddNode->GetThenPin();

		CompilerContext.MovePinLinksToIntermediate(*ContextInputPin, *AddMapValuePin);
		WorkUnitHandleReturnedIdPin->MakeLinkTo(AddMapKeyPin); // AddMapKeyPin->DefaultValue = FString::FromInt(this->GetUniqueID()); // TODO: use WorkUnitHandle GetId for this

		// inject the cache assignment between schedule and bind callback pins by rewiring:
		CompilerContext.MovePinLinksToIntermediate(*BindCallbackNode->GetExecPin(), *AddMapKeyExec);
		AddMapKeyThen->MakeLinkTo(BindCallbackNode->GetExecPin());

		CompilerContext.MessageLog.Note(TEXT("Found ADD Pins @@, @@"), AddMapKeyPin, AddMapValuePin);
	}
	
	// if we have a captured context value, rewire things a bot
	if (bHasValidContext)
	{
		// Get the ID of the work handle pin coming out of our event node
		UK2Node_CallFunction* GetWorkUnitHandleIdNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		GetWorkUnitHandleIdNode->FunctionReference.SetExternalMember(FName("GetWorkUnitHandleId"), UGWBWildcardValueCache::StaticClass());
		GetWorkUnitHandleIdNode->bDefaultsToPureFunc = true;
		GetWorkUnitHandleIdNode->AllocateDefaultPins();
		UEdGraphPin* GetWorkUnitHandleIdInputPin = GetWorkUnitHandleIdNode->FindPin(TEXT("Handle"));
		UEdGraphPin* WorkUnitHandleReturnedIdPin = GetWorkUnitHandleIdNode->FindPin(TEXT("ReturnValue"));
		EventWorkHandlePinOut->MakeLinkTo(GetWorkUnitHandleIdInputPin);
		
		const auto FunctionName_Find = UGWBWildcardValueCache::GetFindFunctionName(ContextType.PinCategory, ContextType.PinSubCategoryObject.Get());
		UK2Node_CallFunction* MapFindNode = CompilerContext.SpawnIntermediateNode<UK2Node_CallFunction>(this, SourceGraph);
		MapFindNode->FunctionReference.SetExternalMember(FName(FunctionName_Find), UGWBWildcardValueCache::StaticClass());
		MapFindNode->AllocateDefaultPins();
		UEdGraphPin* MapFindExecPin = MapFindNode->GetExecPin();
		UEdGraphPin* MapFindKeyPin = MapFindNode->FindPin(TEXT("Key"));
		UEdGraphPin* MapFindReturnValuePin = MapFindNode->FindPin(TEXT("ReturnValue"));
		UEdGraphPin* MapFindThenPin = MapFindNode->GetThenPin();
		WorkUnitHandleReturnedIdPin->MakeLinkTo(MapFindKeyPin); // MapFindKeyPin->DefaultValue = FString::FromInt(this->GetUniqueID())

		CompilerContext.MessageLog.Note(TEXT("Found FIND Pins @@"), MapFindKeyPin);

		// make a sequence node we so we can remove the cached captured value after we invoke the DoWork code
		UK2Node_ExecutionSequence* SequenceNode = CompilerContext.SpawnIntermediateNode<UK2Node_ExecutionSequence>(this, SourceGraph);
		SequenceNode->AllocateDefaultPins();
		SequenceNode->ReconstructNode();
		UEdGraphPin* SequenceExecPin = SequenceNode->GetExecPin();
		UEdGraphPin* FirstSequenceThenPin = SequenceNode->GetThenPinGivenIndex(0);
		UEdGraphPin* SecondSequenceThenPin = SequenceNode->GetThenPinGivenIndex(1);

		// step 2. rewire the context retreival to the context output
		// EventThenPin -> SequenceExecPin -> FirstSequenceThenPin -> MapFindExecPin -> MapFindThenPin
		CompilerContext.MovePinLinksToIntermediate(*EventThenPin, *MapFindThenPin);
		CompilerContext.MovePinLinksToIntermediate(*ContextOutputPin, *MapFindReturnValuePin);
		EventThenPin->MakeLinkTo(SequenceExecPin);
		FirstSequenceThenPin->MakeLinkTo(MapFindExecPin);
	}
	
	// Break any remaining links to our original pins
	BreakAllNodeLinks();
}
UE_ENABLE_OPTIMIZATION

#undef LOCTEXT_NAMESPACE