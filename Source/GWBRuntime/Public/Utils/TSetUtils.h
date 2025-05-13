
#pragma once

#include "CoreMinimal.h"

namespace TSetUtils
{
    /**
     * MAP function for TSet containers
     * Similar to JavaScript Array.map() - creates a new TSet with the results of calling
     * a provided function on every element in the calling TSet.
     * 
     * @param InSet - The input TSet container
     * @param Func - Function to apply to each element (should accept InType and return OutType)
     * @return A new TSet containing the transformed elements
     */
    template<typename InType, typename OutType, typename FunctorType>
    TSet<OutType> Map(const TSet<InType>& InSet, FunctorType Func)
    {
        TSet<OutType> OutSet;
        OutSet.Reserve(InSet.Num());
        
        // Apply the function to each element and add to the output set
        for (const InType& Element : InSet)
        {
            OutSet.Add(Func(Element));
        }
        
        return OutSet;
    }

    /**
     * REDUCE function for TSet containers
     * Similar to JavaScript Array.reduce() - applies a function against an accumulator and
     * each element in the TSet to reduce it to a single value.
     * 
     * @param InSet - The input TSet container
     * @param Func - Reducer function that takes accumulator and current value, returns new accumulator value
     * @param InitialValue - Starting value for the accumulator
     * @return The final accumulated value
     */
    template<typename InType, typename AccumulatorType, typename FunctorType>
    AccumulatorType Reduce(const TSet<InType>& InSet, FunctorType Func, AccumulatorType InitialValue)
    {
        AccumulatorType Accumulator = InitialValue;
        
        // Apply the reducer function to each element
        for (const InType& Element : InSet)
        {
            Accumulator = Func(Accumulator, Element);
        }
        
        return Accumulator;
    }

    /**
     * FILTER function for TSet containers
     * Similar to JavaScript Array.filter() - creates a new TSet with all elements that pass
     * the test implemented by the provided function.
     * 
     * @param InSet - The input TSet container
     * @param Predicate - Function that tests each element (should return boolean)
     * @return A new TSet containing only the elements that passed the test
     */
    template<typename InType, typename PredicateType>
    TSet<InType> Filter(const TSet<InType>& InSet, PredicateType Predicate)
    {
        TSet<InType> OutSet;
        
        // Add only elements that pass the predicate
        for (const InType& Element : InSet)
        {
            if (Predicate(Element))
            {
                OutSet.Add(Element);
            }
        }
        
        return OutSet;
    }
}