# GWBRuntime Module

Contains main functionality of the Gameplay Work Balancer. (budgets, scheduling, job work loop, etc.)

### Relevant Classes
* **UGWBSubsystem** is the engine subsystem to maintain lifecycle of the gameplay work balancer system.
* **UGWBManager** is the main object (intended as singleton) used to schedule and manage work units that should be distributed across frames based on budgets.
* **UGWBScheduler** is responsible for scheduling a delegate on world tick that can be used by the manager to fire the job work loop.
* **FGWBWorkGroupDefinition** is used to define a work group.
* **FGWBWorkGroup** is a runtime instance of a work group that can contain scheduled units of work.
* **FGWBWorkUnit** is an instance of a scheduled unit of work that needs to be done.
* **GWBWorkUnitHandle** is lightweight handle used to react to the DoWork and AbortWork delegates needed by users of this library to actually do their work.

### Configuration
* UGWBManager has a Config property for WorkGroups (FGWBWorkGroupDefinition).
* `CVars.h` has the list of CVars used to control runtime behavior of the GWB system.
* `Stats.h` has the list of performance stats provided by this system.

### Dependencies
* **GWBTimeSlicer Module** - handles low level time slicing and budgeting of for loops.