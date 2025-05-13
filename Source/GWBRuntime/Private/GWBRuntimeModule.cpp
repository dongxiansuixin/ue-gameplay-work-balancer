#include "GWBRuntimeModule.h"

DEFINE_LOG_CATEGORY(Log_GameplayWorkBalancer);

#define LOCTEXT_NAMESPACE "FGWBModule"

void FGWBRuntimeModule::StartupModule()
{
	IModuleInterface::StartupModule();
}

void FGWBRuntimeModule::ShutdownModule()
{
	IModuleInterface::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGWBRuntimeModule, GWBRuntime);