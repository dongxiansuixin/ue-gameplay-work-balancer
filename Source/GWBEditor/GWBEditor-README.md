# GWBEditor Module

Contains K2 node for simplified blueprint usage of the Gameplay Work Balancer.

### Relevant Classes
* `UK2Node_GWBScheduleWorkAsync` use this K2 node to schedule work in blueprints with a simplified interface.


```
  ┌─────────────────────────────────────────────────────────────┐
  │                    Schedule Work Async                      │
  ├─────────────────────────────────────────────────────────────┤
  │                                                             │
  │  ►──── Exec                                                 │
  │  ○──── Context (Wildcard)    ─────────┐                     │
  │  ○──── Work Group (Default)           │                     │
  │  ○──── Work Options                   │                     │
  │                                       │                     │
  │                              ┌────────┴───────────────────► │
  │                              │  On Work Completed ─────►    │
  │                              │  Context (Wildcard) ────○    │
  │                              │  Delta Time (Float) ────○    │
  │                              │  Work Handle ───────────○    │
  │                              └─────────────────────────────►│
  │                                   On Work Aborted  ─────►   │
  │                                                             │
  └─────────────────────────────────────────────────────────────┘

  Key Features:

  - Left Side (Inputs):
    - Exec Pin (►) - White execution arrow
    - Context (○) - Wildcard pin (changes color based on connected type)
    - Work Group (○) - Name pin (purple) with "Default" default value
    - Work Options (○) - Struct pin (blue)
  - Right Side (Outputs):
    - On Work Completed (►) - White execution arrow (top output)
    - On Work Aborted (►) - White execution arrow (bottom output)
    - Context (○) - Wildcard pin (matches input type)
    - Delta Time (○) - Float pin (green)
    - Work Handle (○) - Struct pin (blue)

  Usage Example:
  [Begin Play] ──► [Schedule Work Async] ──► [On Work Completed] ──► [Do Something]
                          │                                               │
                      [MyActor] ──────────────────────────► [MyActor] ────┘
                                                                │
                                                           [0.5 seconds]

  The wildcard pins automatically change color to match whatever type you connect to them (red for
  Actor, yellow for Component, etc.), providing visual feedback about the type being passed through.
```