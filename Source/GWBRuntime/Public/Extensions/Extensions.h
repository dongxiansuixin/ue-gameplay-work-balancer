#pragma once

#include <functional>

enum EBudgetExceededType
{
    FrameTimeBudget = 1,
    UnitCountBudget = 2
};

// Type-erased value modifier using std::function
template<typename ValueType>
class ValueModifier {
public:
    template<typename ImplType>
    ValueModifier(ImplType&& impl)
        : m_ModifyValue([impl = std::forward<ImplType>(impl)](ValueType& value) mutable { 
            impl.ModifyValue(value); 
          })
        , m_OnWorkScheduled([impl = std::forward<ImplType>(impl)](const uint32& totalWorkCount) mutable { 
            impl.OnWorkScheduled(totalWorkCount); 
          })
        , m_OnWorkComplete([impl = std::forward<ImplType>(impl)](const uint32& remainingWorkCount) mutable { 
            impl.OnWorkComplete(remainingWorkCount); 
          })
        , m_OnWorkDeferred([impl = std::forward<ImplType>(impl)](const uint32& remainingWorkCount) mutable { 
            impl.OnWorkDeferred(remainingWorkCount); 
          })
        , m_OnBudgetExceeded([impl = std::forward<ImplType>(impl)](EBudgetExceededType type, const uint32& remainingWorkCount) mutable { 
            impl.OnBudgetExceeded(type, remainingWorkCount); 
          })
    {}

    void ModifyValue(ValueType& value) { m_ModifyValue(value); }
    void OnWorkScheduled(const uint32& totalWorkCount) { m_OnWorkScheduled(totalWorkCount); }
    void OnWorkComplete(const uint32& remainingWorkCount) { m_OnWorkComplete(remainingWorkCount); }
    void OnWorkDeferred(const uint32& remainingWorkCount) { m_OnWorkDeferred(remainingWorkCount); }
    void OnBudgetExceeded(EBudgetExceededType type, const uint32& remainingWorkCount) { m_OnBudgetExceeded(type, remainingWorkCount); }

private:
    std::function<void(ValueType&)> m_ModifyValue;
    std::function<void(const uint32&)> m_OnWorkScheduled;
    std::function<void(const uint32&)> m_OnWorkComplete;
    std::function<void(const uint32&)> m_OnWorkDeferred;
    std::function<void(EBudgetExceededType, const uint32&)> m_OnBudgetExceeded;
};

// Your original template and implementation classes
template<typename ImplType, typename ValueType>
class ValueModifierExtension : public ImplType {
public:
    void ModifyValue(ValueType& Value) { this->ModifyValueImpl(Value); }
    void OnWorkScheduled(const uint32& TotalWorkCount) { this->OnWorkScheduledImpl(TotalWorkCount); }
    void OnWorkComplete(const uint32& RemainingWorkCount) { this->OnWorkCompleteImpl(RemainingWorkCount); }
    void OnWorkDeferred(const uint32& RemainingWorkCount) { this->OnWorkDeferredImpl(RemainingWorkCount); }
    void OnBudgetExceeded(EBudgetExceededType Type, const uint32& RemainingWorkCount) { this->OnBudgetExceededImpl(Type, RemainingWorkCount); }
};

// // Example of usage
// void ExampleUsage() {
//     ModifierManager manager;
//     
//     // Create and add different modifier types
//     manager.AddBudgetModifier(FrameBudgetEscalationModifier());
//     manager.AddBudgetModifier(CountBudgetLimitModifier());
//     
//     // Process all modifiers
//     manager.ProcessBudgetModifiers(42.0);
//     manager.NotifyWorkDeferred();
// }