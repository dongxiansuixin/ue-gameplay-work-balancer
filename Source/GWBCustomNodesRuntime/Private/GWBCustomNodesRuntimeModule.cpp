#include "GWBCustomNodesRuntimeModule.h"

DEFINE_LOG_CATEGORY(LogGWBCustomNodesRuntime);

#define LOCTEXT_NAMESPACE "FGWBCustomNodesRuntimeModule"

void FGWBCustomNodesRuntimeModule::StartupModule()
{
	IModuleInterface::StartupModule();
	
	UE_LOG(LogGWBCustomNodesRuntime, Log, TEXT("GWBCustomNodesRuntime module started"));
}

void FGWBCustomNodesRuntimeModule::ShutdownModule()
{
	UE_LOG(LogGWBCustomNodesRuntime, Log, TEXT("GWBCustomNodesRuntime module shutdown"));
	
	IModuleInterface::ShutdownModule();
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGWBCustomNodesRuntimeModule, GWBCustomNodesRuntime);