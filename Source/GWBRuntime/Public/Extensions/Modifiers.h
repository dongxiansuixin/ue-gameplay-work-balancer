#pragma once

#include "Extensions.h"

/**
 * @brief Modifies the work budget by escalating it each time that there is too much work in a single frame.
 */
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

// type erasure based implementation
using FFrameBudgetEscalationModifier = ValueModifierExtension<FFrameBudgetEscalationModifierImpl, double>;
