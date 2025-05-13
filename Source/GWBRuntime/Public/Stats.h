#pragma once

#include "CoreMinimal.h"

DECLARE_STATS_GROUP(TEXT("GameWorkBalancer"), STATGROUP_GameWorkBalancer, STATCAT_Advanced);

DECLARE_CYCLE_STAT_EXTERN(TEXT("ScheduleWorkUnit"), STAT_ScheduleWorkUnit, STATGROUP_GameWorkBalancer, GWBRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("DoWorkForFrame"), STAT_DoWorkForFrame, STATGROUP_GameWorkBalancer, GWBRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("DoWorkForFrame_Groups"), STAT_DoWorkForFrame_Groups, STATGROUP_GameWorkBalancer, GWBRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("DoWorkForFrame_Reprioritize"), STAT_DoWorkForFrame_Reprioritize, STATGROUP_GameWorkBalancer, GWBRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("DoWorkForGroup"), STAT_DoWorkForGroup, STATGROUP_GameWorkBalancer, GWBRUNTIME_API);
DECLARE_CYCLE_STAT_EXTERN(TEXT("DoWorkForUnit"), STAT_DoWorkForUnit, STATGROUP_GameWorkBalancer, GWBRUNTIME_API);

DECLARE_DWORD_ACCUMULATOR_STAT_EXTERN(TEXT("GameWorkBalancer Work Count"), STAT_GameWorkBalancer_WorkCount, STATGROUP_GameWorkBalancer, GWBRUNTIME_API);
