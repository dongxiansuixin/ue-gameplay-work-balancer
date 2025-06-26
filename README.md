Gameplay Work Balancer (GWB) allows you to define a time budget and schedule units of work that need to be done. The balancer will spread the work (time slice it) across multiple frames so that your game does not exceed it's intended frame budget and maintains a stable frame rate (FPS).

# What the heck is this?

Why do I need the Gameplay Work Balancer?
- Unreal Engine is still single threaded for the purpose of "gameplay" code, most of blueprint code, and code that deals with actor lifecycle.
- You will inevitable run into cases where it's impractical to multi-thread of even optimize the code, but instead you just need to spread the work out to avoid spikey frames and FPS drops.

When should I use this?
- Distrubute spawning VFX across multiple frames for bullet impacts so that visual side-effects don't cause frame spikes.
- Distribute cleanup / destruction of actors across multiple frames so that common gameplay like enemies dying doesn't cause frame spikes.
- Distribute spawning enemies or far away actors across multiple frames. Useful if you have a large batch that needs to spawn but you want to avoid frame spikes.

What is this NOT?
- This system does NOT improve performance... at all. It just distributes work. This implies your game has to run at your target FPS most of the time. If your game is already running at like 15 FPS then distributing the work will prevent it from spiking to 5 FPS but will not make your game run faster.
- This is not a multi-threading framework. Everything still runs on the game thread.

# Getting Started

Schedule some work to spread across frames, call the `ScheduleWork` function and bind a lambda for the work that needs to be done.

```c++
for (auto SpawnEvent : SpawnEnemiesAtLocations)
{
    UGWBManager::ScheduleWork(
        this, // world context object
        "Spawning", // work group
        FGWBWorkOptions::EmptyOptions // opts
    ).OnHandleWork([SpawnEvent](const float DeltaTime){
        // start of work
        auto Enemy = ExpensiveSpawnEnemyFunction(SpawnEvent);
        Enemy.AnotherExpensiveThing();
        // end of work
    })
}
```

What happens from here is:
- The work in your lambda is DEFERRED until the GWB work loop fires (could be start or end of frame).
- The GWB system will process work units one at a time (firing the lambdas) as long as there is time available in the budget (defined via cvar `gwb.budget.frame`)
- If there is sufficient time, your work will complete in the current frame. Otherwise the work is deferred until the next frame and the next time the work loop fires.

You can also do this in Blueprints:

... BP IMAGE EXAMPLE ...

You can disable the system globally by setting the cvar `gwb.enabled` to `false`. When disabled, GWB just immediatelly triggers the work when it's scheduled (as if though it doesn't exist). This is helpful when debugging and you want things to happen in the same stack frame.

## More Advanced Options

- Define the `gwb.budget.count` to add an additional maximum count of work units allowed per frame.
- Use Work Groups to define budgets for specific
- Use `FGWBWorkOptions` properties `MaxDelay` and `MaxNumSkippedFrames` to guarantee work is done within a set number of frames or within a required time window even if it would exceed the budget.
- Both `FGWBWorkOptions` and `FGWBWorkGroupDefinition` have `Priority` settings to control work ordering.
- Abort scheduled work using `UGWBManager::AbortWorkUnit(Handle)`

## Configuration

You can control the system via CVars and INI settings. There are some sensible defaults for most of the configuration but you should likely spend the time to refine for your project.

### Console Variables Reference

| Variable                  | Type  | Default | Description                                                                                                                                                                                                                   |
|---------------------------|-------|---------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| `gwb.enabled`             | bool  | `true`  | Whether balancer is enabled.                                                                                                                                                                                                  |
| `gwb.budget.frame`        | float | `0.005` | Time in seconds balancer may spend per frame doing work (negative values mean infinite budget). It is recommended to customize this budget per platform (i.e. slower platforms may need higher budgets to avoid work delays). |
| `gwb.budget.count`        | int32 | `-1`    | Max number of units of work allowed per work cycle (frame). Negative values mean infinite.                                                                                                                                    |
| `gwb.schedule.interval`      | float | `0.0`   | Time in seconds between balancer work frames, where 0 indicates every frame.                                                                                                                                                  |
| `gwb.immediateduringwork` | bool  | `true`  | Whether work scheduled in the currently working category is immediately executed instead of scheduled for next frame.                                                                                                         |
| `gwb.escalation.scalar`   | float | `0.5`   | Maximum offset scalar to balancer frame budget when escalation triggered, applied as (budget + budget * scalar).                                                                                                              |
| `gwb.escalation.count`    | int32 | `30`    | Number of work instances used as reference for when escalation should be triggered.                                                                                                                                           |
| `gwb.escalation.duration` | float | `0.5`   | How quickly in seconds escalation should scale up.                                                                                                                                                                            |
| `gwb.escalation.decay`    | float | `0.5`   | How quickly in seconds escalation should scale down.                                                                                                                                                                          |

### GameplayWorkGroups INI Configuration

You can define work groups and their budgets in your INI like so:

```ini
[/Script/GWBRuntime.GWBManager]
; High priority work group for critical game systems
+WorkGroupDefinitions=(Id="CriticalSystems",Priority=100,bMutableWhileRunning=true,MaxFrameBudget=0.002,MaxWorkUnitsPerFrame=5,bCanSkipFrame=false,bSkipUnlessFirstInFrame=false,MaxNumSkippedFrames=0,bAlwaysSkipUntilMax=false,SkipPriorityDelta=0)

; Medium priority work group for gameplay systems
+WorkGroupDefinitions=(Id="GameplaySystems",Priority=50,bMutableWhileRunning=false,MaxFrameBudget=0.003,MaxWorkUnitsPerFrame=10,bCanSkipFrame=true,bSkipUnlessFirstInFrame=false,MaxNumSkippedFrames=2,bAlwaysSkipUntilMax=false,SkipPriorityDelta=5)

; AI processing work group with frame skipping capabilities
+WorkGroupDefinitions=(Id="AIProcessing",Priority=30,bMutableWhileRunning=false,MaxFrameBudget=0.004,MaxWorkUnitsPerFrame=8,bCanSkipFrame=true,bSkipUnlessFirstInFrame=false,MaxNumSkippedFrames=3,bAlwaysSkipUntilMax=false,SkipPriorityDelta=10)

; Audio processing work group
+WorkGroupDefinitions=(Id="AudioProcessing",Priority=80,bMutableWhileRunning=true,MaxFrameBudget=0.0015,MaxWorkUnitsPerFrame=15,bCanSkipFrame=false,bSkipUnlessFirstInFrame=false,MaxNumSkippedFrames=0,bAlwaysSkipUntilMax=false,SkipPriorityDelta=0)
```

# Extensions

The Gameplay Work Balancer supports extensions that can change the default behavior. You can register modifiers that mutate the frame budgets or priorority of items before the work loop. We provide one example modifier `FFrameBudgetEscalationModifierImpl` which increases the frame budget by a fixed small value when it's exceeded so that if we do don't have a big work backup but rather a slight FPS drop. The escalation then decays each frame that we don't hit the maximum budget. This grants the system some elasticity to avoid balooning  work unit backlogs.

# FAQ

- **Why did you make this?** When we had to port Godfall, a PS5 title, to work on the PS4, we needed a method to handle the 100s of blueprints and gameplay effects that caused frame spikes here and there but most of the time were not using up a lot of time. We found that in 80% of the cases it was totally fine to let some of this work happen a frame or two later.
- **I'm using this GWB thing and my game is still slow! What gives?** The GWB does not improve performance. It simply distributes your existing poorly performing code over multiple frames to prevent FPS drops. This ONLY works if you have room in your frame budget (i.e. your game is running at like 90 FPS most of the time, and then has moments where it drops to 20 because of some heavy mass actor spawns or similar spiky gameplay code).
- **Does this use multiple threads?** No. Everything still runs on the game thread.
- **Shouldn't I just use multiple threads?** You can't multi-thead a lot of gameplay code (actor lifecycle) and blueprint code, and multi-threading is overkill for having a small frame spike here and there.
- **I heard ECS is great, I can just use that right?** You could totally use ECS to defer work and roll your own system to manage jobs across frames. The GWB kind of does that without ECS and is intended to be peppered throughout standard unreal gameplay code (actor land).
- **Should I use this for all my gameplay stuff?** Probably not. Use sparingly when you need to optimize some specific part of your game that causes frame spikes.

# Related Similar Implementations

**LatentTickTimeBudget in ue5coro** [Link](https://github.com/landelare/ue5coro/blob/master/Docs/LatentTickTimeBudget.md) - this is a great tool if you don't need a global manager, groups, and other bells and whistles (the time slicers module in this repo has sort of a similar API).

# Under the Hood

### Time Slicers

Inside the module **GWBTimeSlicer** you will find some useful utilities for simplified budget and time management.

```c++
void MyFunctionWithABigExpensiveLoop()
{
  // time we're allowed to spend per frame
  static const float FrameBudget = 0.05f;
  // max loop iterations per frame
  static const int MaxCountAllowedPerFrame = 100;

  // resets used budgets when it goes out of scope
  FGWBTimeSliceScopedHandle TimeSlicer(this, FName("OverlapsSlicer"), FrameBudget, MaxCountAllowedPerFrame);

  for (auto Overlap : OverlapsList)
  {
    // increments frame budget usage when it goes out of scope
    FGWBTimeSlicedLoopScope TimeSlicedWork(this, FName("OverlapsSlicer"), FrameBudget, MaxCountAllowedPerFrame);
    // if we're out of budget break the loop
    if (TimeSlicedWork.IsOverBudget()) break;
    // do your expensive work
    DoSomethingExpensive(Overlap);
  }
}
```

### Budgeted For Loop

Under the hood uses time slicers and wraps it in a macro.

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