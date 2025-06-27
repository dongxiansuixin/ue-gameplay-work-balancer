#include "GWBEditorModule.h"

#include "EdGraphUtilities.h"
#include "BlueprintGraph/GWBGraphPanelPinFactory.h"

#define LOCTEXT_NAMESPACE "FGWBEditorModule"

void FGWBEditorModule::StartupModule()
{
	// Register factories for pins and nodes
	GwbGraphPanelPinFactory = MakeShareable(new FGWBGraphPanelPinFactory());
	FEdGraphUtilities::RegisterVisualPinFactory(GwbGraphPanelPinFactory);
}

void FGWBEditorModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGWBEditorModule, GWBEditor)