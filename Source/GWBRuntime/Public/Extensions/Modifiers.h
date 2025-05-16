#pragma once

#include "Extensions.h"

class FFrameBudgetEscalationModifierImpl {
private:
	double EscalationScalar = 0;
	double LastEscalationUpdateTimestamp = 0;
	uint32 TotalNumWorkInstances = 0;
protected:
	void ModifyValueImpl(double& Value);
	void OnWorkScheduledImpl(const uint32& TotalWorkCount);
	void OnWorkCompleteImpl(const uint32& RemainingWorkCount);
	FORCEINLINE void OnWorkDeferredImpl(const uint32& RemainingWorkCount) {}
	FORCEINLINE void OnBudgetExceededImpl(EBudgetExceededType Type, const uint32& RemainingWorkCount) {}
};

// Type definitions for concrete implementations
using FFrameBudgetEscalationModifier = ValueModifierExtension<FFrameBudgetEscalationModifierImpl, double>;
