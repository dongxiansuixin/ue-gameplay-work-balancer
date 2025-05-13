// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

GWBRUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(Log_GameplayWorkBalancer, Log, All);

class FGWBRuntimeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
