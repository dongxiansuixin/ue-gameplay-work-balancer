// ModifierManager.h
#pragma once

#include "CoreMinimal.h"
#include "Extensions.h"

class UGWBManager;

/**
 * @brief Acts as an extension point for modifying budgets (etc.) based on various rules.
 * You can implement a modifier for budget or priority and register it with this manager.
 * Then, once budgets are calculated, this manager is meant to be invoked via `ProcessBudgetModifiers()` (and similar)
 * to modify the final values based on additional (optional) rules that can be defined by your own project.
 *
 * (author's note: this whole system is a bit underbaked, but at least in principle it's fast since it doesn't
 * use inheritance or virtual method tables but relies on type erasure instead... this means it's never going to be
 * BP compatible.)
 */
class FModifierManager {
  friend UGWBManager;

public:
  FModifierManager() = default;
  ~FModifierManager() = default;

  // Disable copy operations
  FModifierManager(const FModifierManager&) = delete;
  FModifierManager& operator=(const FModifierManager&) = delete;

  // Allow move operations
  FModifierManager(FModifierManager&&) = default;
  FModifierManager& operator=(FModifierManager&&) = default;

  // Reset the modifier lists
  void Reset()
  {
      m_BudgetModifiers.Empty();
      m_PriorityModifiers.Empty();
  }

  void AddBudgetModifier(ValueModifier<double> modifier) {
      m_BudgetModifiers.Add(MoveTemp(modifier));
  }

  void AddPriorityModifier(ValueModifier<double> modifier) {
      m_PriorityModifiers.Add(MoveTemp(modifier));
  }

protected:

  void ProcessBudgetModifiers(double& value) {
      for (auto& modifier : m_BudgetModifiers) {
          modifier.ModifyValue(value);
      }
  }

  void ProcessPriorityModifiers(double& value) {
      for (auto& modifier : m_PriorityModifiers) {
          modifier.ModifyValue(value);
      }
  }

  void NotifyWorkScheduled(const uint32& TotalWorkCount) {
      for (auto& modifier : m_BudgetModifiers) {
          modifier.OnWorkScheduled(TotalWorkCount);
      }
      for (auto& modifier : m_PriorityModifiers) {
          modifier.OnWorkScheduled(TotalWorkCount);
      }
  }

  void NotifyWorkDeferred(const uint32& RemainingWorkCount) {
      for (auto& modifier : m_BudgetModifiers) {
          modifier.OnWorkDeferred(RemainingWorkCount);
      }
      for (auto& modifier : m_PriorityModifiers) {
          modifier.OnWorkDeferred(RemainingWorkCount);
      }
  }

  void NotifyWorkComplete(const uint32& RemainingWorkCount) {
      for (auto& modifier : m_BudgetModifiers) {
          modifier.OnWorkComplete(RemainingWorkCount);
      }
      for (auto& modifier : m_PriorityModifiers) {
          modifier.OnWorkComplete(RemainingWorkCount);
      }
  }

  void NotifyBudgetExceeded(EBudgetExceededType Type, const uint32& RemainingWorkCount) {
      for (auto& modifier : m_BudgetModifiers) {
          modifier.OnBudgetExceeded(Type, RemainingWorkCount);
      }
      for (auto& modifier : m_PriorityModifiers) {
          modifier.OnBudgetExceeded(Type, RemainingWorkCount);
      }
  }

private:
  TArray<ValueModifier<double>> m_BudgetModifiers;
  TArray<ValueModifier<double>> m_PriorityModifiers;
};