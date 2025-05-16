#pragma once

static TAutoConsoleVariable<bool> CVarGWB_Enabled(TEXT("gwb.enabled"), true, TEXT("Whether balancer is enabled."));
static TAutoConsoleVariable<float> CVarGWB_FrameBudget(TEXT("gwb.frame.budget"), 0.005, TEXT("Time in seconds balancer may spend per frame doing work. It is recommended to customize this budget per platform (i.e. slower platforms may need higher budgets to avoid work delays)."));
static TAutoConsoleVariable<float> CVarGWB_FrameInterval(TEXT("gwb.frame.interval"), 0.0, TEXT("Time in seconds between balancer work frames, where 0 indicates every frame."));

static TAutoConsoleVariable<bool> CVarGWB_ImmediateDuringWork(TEXT("gwb.immediateduringwork"), true, TEXT("Whether work scheduled in the currently working category is immediately executed instead of scheduled for next frame."));

static TAutoConsoleVariable<float> CVarGWB_EscalationScalar(TEXT("gwb.escalation.scalar"), 0.5, TEXT("Maximum offset scalar to balancer frame budget when escalation triggered, applied as (budget + budget * scalar)."));
static TAutoConsoleVariable<int32> CVarGWB_EscalationCount(TEXT("gwb.escalation.count"), 30, TEXT("Number of work instances used as reference for when escalation should be triggered."));
static TAutoConsoleVariable<float> CVarGWB_EscalationDuration(TEXT("gwb.escalation.duration"), 0.5, TEXT("How quickly in seconds escalation should scale up."));
static TAutoConsoleVariable<float> CVarGWB_EscalationDecay(TEXT("gwb.escalation.decay"), 0.5, TEXT("How quickly in seconds escalation should scale down."));
