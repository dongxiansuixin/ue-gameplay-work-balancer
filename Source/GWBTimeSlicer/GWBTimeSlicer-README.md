# GWBTimeSlicer Module

Contains functionality and tools for low level time slicing (distributing work across frames) of loops.

### Relevant Classes
* `UGWBTimeSlicer` is a minimal tracker of time, budget, and telemetry of a time slicing operation.
* `UGWBTimeSlicersSubsystem` manages a global registry of time slicers indexed by identifier. The slicers (unfortnuately at the moment) never get garbage collected.
* `FGWBTimeSlicedScope` 
* `FGWBTimeSlicedLoopScope` 

### Basic Usage
* Declare a `FGWBTimeSlicedScope` at the top of your function or outer scope.
* Declare a `FGWBTimeSlicedLoopScope` at the top of your for-loop scope.
* Check `TimeSlicedWork.IsOverBudget()` inside the loop and break if needed.

```c++
void MyFunctionWithABigExpensiveLoop()
{
  // time we're allowed to spend per frame
  static const float FrameBudget = 0.05f;
  // max loop iterations per frame
  static const int MaxCountAllowedPerFrame = 100;

  // resets used budgets when it goes out of scope
  FGWBTimeSlicedScope TimeSlicer(this, FName("OverlapsSlicer"), FrameBudget, MaxCountAllowedPerFrame);

  for (auto Overlap : OverlapsList)
  {
    // increments frame budget usage when it goes out of scope
    FGWBTimeSlicedLoopScope TimeSlicedLoop = TimeSlicer.StartLoopScope();
    // if we're out of budget break the loop
    if (TimeSlicedLoop.IsOverBudget()) break;
    // do your expensive work
    DoSomethingExpensive(Overlap);
  }
}
```

### Helpful Utilities
* `BUDGETED_FOR_LOOP` this macro give you a tool to make a for frame budget aware for-loop that can spread across multiple frames.

```c++
BUDGETED_FOR_LOOP(
    this,    // context object
    0.1f,    // frame budget
    10,      // max count of loop iterations per frame
    MyArray, 
    [&](FBudgetedLoopHandle& Handle
) {
    // Your expensive processing code here
    if (SomeCondition)
    {
        Handle.Break(); // Exit loop early
        return;
    }
});
```