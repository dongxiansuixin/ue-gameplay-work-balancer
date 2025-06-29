

#pragma once

GWBRUNTIME_API DECLARE_LOG_CATEGORY_EXTERN(Log_GameplayWorkBalancer, Log, All);

class FGWBRuntimeModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
