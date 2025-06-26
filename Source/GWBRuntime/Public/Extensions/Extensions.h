#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"

enum EBudgetExceededType
{
  FrameTimeBudget = 1,
  UnitCountBudget = 2
};

// Type-erased value modifier using TFunction
template<typename ValueType>
class ValueModifier {
public:
  template<typename ImplType>
  ValueModifier(ImplType&& impl)
  {
    // Create a shared implementation object
    auto sharedImpl = MakeShared<ImplType>(MoveTemp(impl));

    m_ModifyValue = [sharedImpl](ValueType& value) {
      	sharedImpl->ModifyValue(value);
    };

    m_OnWorkScheduled = [sharedImpl](const uint32& totalWorkCount) {
      	sharedImpl->OnWorkScheduled(totalWorkCount);
    };

    m_OnWorkComplete = [sharedImpl](const uint32& remainingWorkCount) {
      	sharedImpl->OnWorkComplete(remainingWorkCount);
    };

    m_OnWorkDeferred = [sharedImpl](const uint32& remainingWorkCount) {
      	sharedImpl->OnWorkDeferred(remainingWorkCount);
    };

    m_OnBudgetExceeded = [sharedImpl](EBudgetExceededType type, const uint32& remainingWorkCount) {
      	sharedImpl->OnBudgetExceeded(type, remainingWorkCount);
    };
  }

  // Allow move construction
  ValueModifier(ValueModifier&& Other) = default;

  // Allow move assignment
  ValueModifier& operator=(ValueModifier&& Other) = default;

  // Disallow copying
  ValueModifier(const ValueModifier&) = delete;
  ValueModifier& operator=(const ValueModifier&) = delete;

  void ModifyValue(ValueType& value) { m_ModifyValue(value); }
  void OnWorkScheduled(const uint32& totalWorkCount) { m_OnWorkScheduled(totalWorkCount); }
  void OnWorkComplete(const uint32& remainingWorkCount) { m_OnWorkComplete(remainingWorkCount); }
  void OnWorkDeferred(const uint32& remainingWorkCount) { m_OnWorkDeferred(remainingWorkCount); }
  void OnBudgetExceeded(EBudgetExceededType type, const uint32& remainingWorkCount) { m_OnBudgetExceeded(type, remainingWorkCount); }

private:
  TFunction<void(ValueType&)> m_ModifyValue;
  TFunction<void(const uint32&)> m_OnWorkScheduled;
  TFunction<void(const uint32&)> m_OnWorkComplete;
  TFunction<void(const uint32&)> m_OnWorkDeferred;
  TFunction<void(EBudgetExceededType, const uint32&)> m_OnBudgetExceeded;
};

// Your original template and implementation classes
template<typename ImplType, typename ValueType>
class ValueModifierExtension : public ImplType {
public:
  void ModifyValue(ValueType& Value) { this->ModifyValueImpl(Value); }
  void OnWorkComplete(const uint32& RemainingWorkCount) { this->OnWorkCompleteImpl(RemainingWorkCount); }
  void OnWorkDeferred(const uint32& RemainingWorkCount) { this->OnWorkDeferredImpl(RemainingWorkCount); }
  void OnBudgetExceeded(EBudgetExceededType Type, const uint32& RemainingWorkCount) { this->OnBudgetExceededImpl(Type, RemainingWorkCount); }
};