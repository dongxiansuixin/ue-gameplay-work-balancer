#include "DataTypes/GWBWorkUnitHandle.h"
#include "GWBRuntimeModule.h"

void FGWBWorkUnitHandle::OnHandleWork(TFunction<void(const float DeltaTime)> DispatchOnDoWork) const
{
	if (bShouldAutoFire)
	{
		DispatchOnDoWork(0);
	} else
	{
		FCriticalSection CriticalSection;
		{
			FScopeLock Lock(&CriticalSection);
			GetWorkCallback().BindLambda([DispatchOnDoWork](float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				DispatchOnDoWork(DeltaTime);
			});
		}
	}
}
