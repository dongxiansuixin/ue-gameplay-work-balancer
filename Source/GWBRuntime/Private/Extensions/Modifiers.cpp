#include "Extensions/Modifiers.h"
#include "Cvars.h"

void FFrameBudgetEscalationModifierImpl::ModifyValueImpl(double& Value)
{
	const auto Now = FPlatformTime::Seconds();
	const auto DeltaTime = Now - LastEscalationUpdateTimestamp;
	if (TotalNumWorkInstances > (uint32)CVarGWB_EscalationCount.GetValueOnGameThread())
	{
		const double EscalationDeltaPerSec = (double)(CVarGWB_EscalationScalar.GetValueOnGameThread() / CVarGWB_EscalationDuration.GetValueOnGameThread());
		EscalationScalar = FMath::Min(EscalationScalar + EscalationDeltaPerSec * DeltaTime, (double)CVarGWB_EscalationScalar.GetValueOnGameThread());
	}
	else if (EscalationScalar != 0.0)
	{
		const double EscalationDeltaPerSec = (double)(CVarGWB_EscalationScalar.GetValueOnGameThread() / CVarGWB_EscalationDecay.GetValueOnGameThread());
		EscalationScalar = FMath::Max(EscalationScalar - EscalationDeltaPerSec * DeltaTime, 0.0);
	}
	Value += Value * EscalationScalar;
	LastEscalationUpdateTimestamp = Now;
}

void FFrameBudgetEscalationModifierImpl::OnWorkScheduledImpl(const uint32& TotalWorkCount)
{
	TotalNumWorkInstances = TotalWorkCount;
}

void FFrameBudgetEscalationModifierImpl::OnWorkCompleteImpl(const uint32& RemainingWorkCount)
{
	TotalNumWorkInstances = RemainingWorkCount;
}
