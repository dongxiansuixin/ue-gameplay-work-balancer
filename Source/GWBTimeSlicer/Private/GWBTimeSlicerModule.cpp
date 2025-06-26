#include "GWBTimeSlicerModule.h"

DEFINE_LOG_CATEGORY(Log_GWBTimeSlicer);

#define LOCTEXT_NAMESPACE "FGWBTimeSlicerModule"

class FGWBTimeSlicerModule : public IGWBTimeSlicerModule
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

void FGWBTimeSlicerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file
}

void FGWBTimeSlicerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FGWBTimeSlicerModule, GWBTimeSlicer)