﻿#pragma once

// core system
static TAutoConsoleVariable<bool> CVarGWB_Enabled(TEXT("gwb.enabled"), true, TEXT("Whether balancer is enabled."));
static TAutoConsoleVariable<float> CVarGWB_FrameBudget(TEXT("gwb.budget.frame"), 0.005, TEXT("Time in seconds balancer may spend per frame doing work (negative values mean infnite budget). It is recommended to customize this budget per platform (i.e. slower platforms may need higher budgets to avoid work delays)."));
static TAutoConsoleVariable<int32> CVarGWB_WorkCountBudget(TEXT("gwb.budget.count"), -1, TEXT("Max number of units of work allowed per work cycle (frame). Negative values mean infinite."));
static TAutoConsoleVariable<float> CVarGWB_FrameInterval(TEXT("gwb.schedule.interval"), 0.0, TEXT("Time in seconds between balancer work frames, where 0 indicates every frame."));

// not yet implemented
static TAutoConsoleVariable<bool> CVarGWB_ImmediateDuringWork(TEXT("gwb.immediateduringwork"), true, TEXT("Whether work scheduled in the currently working category is immediately executed instead of scheduled for next frame."));

// escalation extension
static TAutoConsoleVariable<float> CVarGWB_EscalationScalar(TEXT("gwb.escalation.scalar"), 0.5, TEXT("Maximum offset scalar to balancer frame budget when escalation triggered, applied as (budget + budget * scalar)."));
static TAutoConsoleVariable<int32> CVarGWB_EscalationCount(TEXT("gwb.escalation.count"), 30, TEXT("Number of work instances used as reference for when escalation should be triggered."));
static TAutoConsoleVariable<float> CVarGWB_EscalationDuration(TEXT("gwb.escalation.duration"), 0.5, TEXT("How quickly in seconds escalation should scale up."));
static TAutoConsoleVariable<float> CVarGWB_EscalationDecay(TEXT("gwb.escalation.decay"), 0.5, TEXT("How quickly in seconds escalation should scale down."));
