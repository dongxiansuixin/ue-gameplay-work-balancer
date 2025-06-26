#pragma once

#include "CoreMinimal.h"
#include "HAL/IConsoleManager.h"

/**
 * Templated helper class that sets a console variable to a new value,
 * then restores it to its original value when the object goes out of scope.
 * 
 * Supports int32, float, bool, and FString types.
 */
template<typename T>
class FScopedCVarOverride
{
public:
    /**
     * Constructor - finds the console variable, saves its current value, and sets it to the new value
     * 
     * @param VariableName - Name of the console variable to override
     * @param Value - New value to set the console variable to
     */
    FScopedCVarOverride(const TCHAR* VariableName, T Value)
        : Variable(IConsoleManager::Get().FindConsoleVariable(VariableName))
    {
        if (Variable)
        {
            SaveValue();
            SetValue(Value);
        }
    }

    /** Destructor - restores the console variable to its original value */
    ~FScopedCVarOverride()
    {
        if (Variable)
        {
            RestoreValue();
        }
    }

    // Deleted copy/move constructors and assignment operators
    FScopedCVarOverride(FScopedCVarOverride&&) = delete;
    FScopedCVarOverride(const FScopedCVarOverride&) = delete;
    FScopedCVarOverride& operator=(FScopedCVarOverride&&) = delete;
    FScopedCVarOverride& operator=(const FScopedCVarOverride&) = delete;

    /**
     * Sets the console variable to a new value
     * 
     * @param NewValue - New value to set
     */
    void Set(T NewValue)
    {
        if (Variable)
        {
            SetValue(NewValue);
        }
    }

    /**
     * Restores the console variable to its original value
     */
    void Restore()
    {
        if (Variable)
        {
            RestoreValue();
        }
    }

    /**
     * Checks if the console variable was found
     * 
     * @return true if the console variable was found, false otherwise
     */
    bool IsValid() const
    {
        return Variable != nullptr;
    }

private:
    IConsoleVariable* Variable = nullptr;
    T SavedValue = T();

    // Type-specific implementations are provided in specializations
    void SaveValue();
    void SetValue(T Value);
    void RestoreValue();
};

// Specialization for int32
template<>
inline void FScopedCVarOverride<int32>::SaveValue()
{
    SavedValue = Variable->GetInt();
}

template<>
inline void FScopedCVarOverride<int32>::SetValue(int32 Value)
{
    Variable->Set(Value);
}

template<>
inline void FScopedCVarOverride<int32>::RestoreValue()
{
    Variable->Set(SavedValue);
}

// Specialization for float
template<>
inline void FScopedCVarOverride<float>::SaveValue()
{
    SavedValue = Variable->GetFloat();
}

template<>
inline void FScopedCVarOverride<float>::SetValue(float Value)
{
    Variable->Set(Value);
}

template<>
inline void FScopedCVarOverride<float>::RestoreValue()
{
    Variable->Set(SavedValue);
}

// Specialization for bool
template<>
inline void FScopedCVarOverride<bool>::SaveValue()
{
    SavedValue = Variable->GetBool();
}

template<>
inline void FScopedCVarOverride<bool>::SetValue(bool Value)
{
    Variable->Set(Value ? 1 : 0);
}

template<>
inline void FScopedCVarOverride<bool>::RestoreValue()
{
    Variable->Set(SavedValue ? 1 : 0);
}

// Specialization for FString
template<>
class FScopedCVarOverride<FString>
{
public:
    FScopedCVarOverride(const TCHAR* VariableName, const FString& Value)
        : Variable(IConsoleManager::Get().FindConsoleVariable(VariableName))
    {
        if (Variable)
        {
            SavedValue = Variable->GetString();
            Variable->Set(*Value);
        }
    }

    ~FScopedCVarOverride()
    {
        if (Variable)
        {
            Variable->Set(*SavedValue);
        }
    }

    FScopedCVarOverride(FScopedCVarOverride&&) = delete;
    FScopedCVarOverride(const FScopedCVarOverride&) = delete;
    FScopedCVarOverride& operator=(FScopedCVarOverride&&) = delete;
    FScopedCVarOverride& operator=(const FScopedCVarOverride&) = delete;

    void Set(const FString& NewValue)
    {
        if (Variable)
        {
            Variable->Set(*NewValue);
        }
    }

    void Restore()
    {
        if (Variable)
        {
            Variable->Set(*SavedValue);
        }
    }

    bool IsValid() const
    {
        return Variable != nullptr;
    }

private:
    IConsoleVariable* Variable = nullptr;
    FString SavedValue;
};

// Convenience typedefs
using FScopedCVarOverrideInt = FScopedCVarOverride<int32>;
using FScopedCVarOverrideFloat = FScopedCVarOverride<float>;
using FScopedCVarOverrideBool = FScopedCVarOverride<bool>;
using FScopedCVarOverrideString = FScopedCVarOverride<FString>;