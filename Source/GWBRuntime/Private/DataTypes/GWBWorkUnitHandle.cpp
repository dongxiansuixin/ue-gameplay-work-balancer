#include "DataTypes/GWBWorkUnitHandle.h"
#include "GWBRuntimeModule.h"

void FGWBWorkUnitHandle::OnHandleWork(TFunction<void(const float TimeSinceScheduled, const FGWBWorkUnitHandle& Handle)> DispatchOnDoWork) const
{
	if (bShouldAutoFire)
	{
		DispatchOnDoWork(0, FGWBWorkUnitHandle());
	} else
	{
		FCriticalSection CriticalSection;
		{
			FScopeLock Lock(&CriticalSection);
			GetWorkCallback().BindLambda([DispatchOnDoWork](float TimeSinceScheduled, const FGWBWorkUnitHandle& Handle)
			{
				DispatchOnDoWork(TimeSinceScheduled, Handle);
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
