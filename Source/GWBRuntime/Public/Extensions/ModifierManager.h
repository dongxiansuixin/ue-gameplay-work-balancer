#pragma once

#include "Extensions.h"

class UGWBManager;

class FModifierManager {

	friend UGWBManager;
	
public:

	void AddBudgetModifier(ValueModifier<double> modifier) {
		m_BudgetModifiers.push_back(std::move(modifier));
	}
    
	void AddPriorityModifier(ValueModifier<double> modifier) {
		m_PriorityModifiers.push_back(std::move(modifier));
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
	std::vector<ValueModifier<double>> m_BudgetModifiers;
	std::vector<ValueModifier<double>> m_PriorityModifiers;
};
