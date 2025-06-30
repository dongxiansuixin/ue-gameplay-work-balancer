#include "DataTypes/GWBWorkUnitHandle.h"
#include "GWBRuntimeModule.h"

void FGWBWorkUnitHandle::OnHandleWork(TFunction<void(const float DeltaTime, const FGWBWorkUnitHandle& Handle)> DispatchOnDoWork) const
{
	if (bShouldAutoFire)
	{
		DispatchOnDoWork(0, FGWBWorkUnitHandle());
	} else
	{
		FCriticalSection CriticalSection;
		{
			FScopeLock Lock(&CriticalSection);
			GetWorkCallback().BindLambda([DispatchOnDoWork](float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				DispatchOnDoWork(DeltaTime, Handle);
			});
		}
	}
}

void FGWBWorkUnitHandle::OnHandleWork(TFunction<void()> DispatchOnDoWork) const
{
	OnHandleWork([DispatchOnDoWork](const float, const FGWBWorkUnitHandle&)
	{
		DispatchOnDoWork();
	});
}
