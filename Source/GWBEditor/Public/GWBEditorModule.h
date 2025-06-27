#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FGWBGraphPanelPinFactory;

class FGWBEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
private:
	TSharedPtr<FGWBGraphPanelPinFactory> GwbGraphPanelPinFactory;
};
