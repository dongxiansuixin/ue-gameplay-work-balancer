

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleInterface.h"

GWBCUSTOMNODESRUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(LogGWBCustomNodesRuntime, Log, All);

class FGWBCustomNodesRuntimeModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};