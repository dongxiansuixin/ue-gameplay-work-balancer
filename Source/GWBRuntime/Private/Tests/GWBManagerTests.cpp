﻿
#include "DataTypes/GWBWorkUnitHandle.h"
#include "GWBRuntimeModule.h"
#include "Misc/AutomationTest.h"
#include "GWBManager.h"
#include "Tests/RuntimeTestMocks.h"
#include "Tests/ScopedCvarOverrides.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FGWBManagerTests, "GWBRuntime.GWBManager", EAutomationTestFlags::ProductFilter | EAutomationTestFlags_ApplicationContextMask)
	// add any member vars here
	UGWBManagerMock* Manager;
	FName WorkGroupID;
	FGWBWorkGroupDefinition WorkGroupDefinition;
	void PrepareTests();
END_DEFINE_SPEC(FGWBManagerTests)
void FGWBManagerTests::PrepareTests()
{
	BeforeEach([this]()
	{
		WorkGroupDefinition = FGWBWorkGroupDefinition();
		WorkGroupID = FName("TestGroup");
		WorkGroupDefinition.Id = WorkGroupID;
		Manager = FGWBManagerTestHelper::Create();
		Manager->WorkGroups.Add(FGWBWorkGroup(WorkGroupDefinition));
	});
	AfterEach([this]()
	{
		if (Manager->IsValidLowLevel()) Manager->ConditionalBeginDestroy();
	});
}

UE_DISABLE_OPTIMIZATION
void FGWBManagerTests::Define()
{
	
	Describe("ScheduleWork()", [this]()
	{
		PrepareTests();
		It("should not schedule anything if balancer is disabled via CVAR", [this]()
		{
			FScopedCVarOverrideBool CvarEnabled(TEXT("gwb.enabled"), false);
			bool bCallbackFired = false;
			auto Handle = Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Handle.OnHandleWork([&bCallbackFired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				bCallbackFired = true;
			});
			TestTrue("Passthrough callback fired", bCallbackFired);
			TestTrue("# of scheduled work units is 0", Manager->TEST_GetWorkUnitCount() == 0);
		});
 
		It("should schedule work into correct group when enabled", [this]()
		{
			FScopedCVarOverrideBool CvarEnabled(TEXT("gwb.enabled"), true);
			bool bCallbackFired = false;
			auto Handle = Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Handle.OnHandleWork([&bCallbackFired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				bCallbackFired = true;
			});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			TestFalse("Callback should NOT be fired since we did not do work", bCallbackFired);
			TestTrue("# of scheduled work units is 2", Manager->TEST_GetWorkUnitCount() == 2);
		});
	});
	
	Describe("DoWork() - Basics", [this]()
	{
		PrepareTests();

		It("should perform work if budget is negative, since NEGATIVE VALUES are treated as disabled", [this]()
		{
			FScopedCVarOverrideFloat CvarFrameBudget(TEXT("gwb.budget.frame"), -1);
			bool bCallbackFired = false;
			auto Handle = Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Handle.OnHandleWork([&bCallbackFired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				bCallbackFired = true;
			});
			Manager->DoWork();
			TestTrue("Callback should be fired since we did not a real budget", bCallbackFired);
			TestTrue("# of scheduled work units is 0", Manager->TEST_GetWorkUnitCount() == 0);
		});
		
		It("should NOT perform any units of work if there's no budget", [this]()
		{
			FScopedCVarOverrideFloat CvarFrameBudget(TEXT("gwb.budget.frame"), 0);
			bool bCallbackFired = false;
			auto Handle = Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Handle.OnHandleWork([&bCallbackFired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				bCallbackFired = true;
			});
			Manager->DoWork();
			TestFalse("Callback should NOT be fired since we did not have sufficient budget", bCallbackFired);
			TestTrue("# of scheduled work units is 1", Manager->TEST_GetWorkUnitCount() == 1);
		});
		
		It("should perform a unit of work if there's budget", [this]()
		{
			FScopedCVarOverrideFloat CvarFrameBudget(TEXT("gwb.budget.frame"), 0.1f);
			bool bCallbackFired = false;
			auto Handle = Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Handle.OnHandleWork([&bCallbackFired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				bCallbackFired = true;
			});
			Manager->DoWork();
			TestTrue("Callback should have fired", bCallbackFired);
			TestTrue("# of scheduled work units is 1", Manager->TEST_GetWorkUnitCount() == 0);
		});
		
		It("should NOT perform a unit of work after budget is exhausted", [this]()
		{
			FScopedCVarOverrideFloat CvarFrameBudget(TEXT("gwb.budget.frame"), 0.1f);
			bool bCallbackFired = false;
			bool bCallback2Fired = false;
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false}).OnHandleWork([&bCallbackFired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				FPlatformProcess::Sleep(0.1);
				bCallbackFired = true;
			});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false}).OnHandleWork([&bCallback2Fired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				FPlatformProcess::Sleep(0.1);
				bCallback2Fired = true;
			});
			Manager->DoWork();
			TestTrue("Callback 1 should have fired", bCallbackFired);
			TestTrue("Callback 2 should NOT have fired", !bCallback2Fired);
			TestTrue("# of scheduled work units is 1", Manager->TEST_GetWorkUnitCount() == 1);
		});
		
		It("should clear work over two iterations if budget exhausted on first iteration", [this]()
		{
			FScopedCVarOverrideFloat CvarFrameBudget(TEXT("gwb.budget.frame"), 0.1f);
			bool bCallbackFired = false;
			bool bCallback2Fired = false;
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false}).OnHandleWork([&bCallbackFired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				FPlatformProcess::Sleep(0.1);
				bCallbackFired = true;
			});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false}).OnHandleWork([&bCallback2Fired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				FPlatformProcess::Sleep(0.1);
				bCallback2Fired = true;
			});
			Manager->DoWork();
			TestTrue("Callback 1 should have fired", bCallbackFired);
			TestTrue("Callback 2 should NOT have fired", !bCallback2Fired);
			TestTrue("# of scheduled work units is 1", Manager->TEST_GetWorkUnitCount() == 1);
			Manager->DoWork();
			TestTrue("Callback 1 should have fired", bCallbackFired);
			TestTrue("Callback 2 should have fired", bCallback2Fired);
			TestTrue("# of scheduled work units is 0", Manager->TEST_GetWorkUnitCount() == 0);
		});
		
		It("should perform work in order of priority", [this]()
		{
			FScopedCVarOverrideFloat CvarFrameBudget(TEXT("gwb.budget.frame"), 0.1f);
			bool bCallbackLastFired = false;
			Manager->ScheduleWork( WorkGroupID, { 1, 0, 0, false, false}).OnHandleWork([&bCallbackLastFired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				FPlatformProcess::Sleep(0.1);
				bCallbackLastFired = true;
			});
			bool bCallbackFirstFired = false;
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false}).OnHandleWork([&bCallbackFirstFired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				FPlatformProcess::Sleep(0.1);
				bCallbackFirstFired = true;
			});
			Manager->DoWork();
			TestTrue("Callback 1 should have fired", bCallbackFirstFired);
			TestFalse("Callback 2 should NOT have fired", bCallbackLastFired);
			Manager->DoWork();
			TestTrue("Callback 2 should have fired", bCallbackLastFired);
			TestTrue("# of scheduled work units is 0", Manager->TEST_GetWorkUnitCount() == 0);
		});
	});
	
	Describe("DoWork() - Group Budget", [this]()
	{
		BeforeEach([this]()
		{
			const FName WorkGroupA = FName("WorkGroupA");
			const FName WorkGroupB = FName("WorkGroupB");
			Manager = FGWBManagerTestHelper::Create();
			// add group A
			FGWBWorkGroupDefinition WorkGroupDefinitionA = FGWBWorkGroupDefinition();
			WorkGroupDefinitionA.Id = WorkGroupA;
			WorkGroupDefinitionA.Priority = 0;
			WorkGroupDefinitionA.MaxFrameBudget = 0.1;
			WorkGroupDefinitionA.MaxWorkUnitsPerFrame = 3;
			Manager->WorkGroups.Add(FGWBWorkGroup(WorkGroupDefinitionA));
			// add group B
			FGWBWorkGroupDefinition WorkGroupDefinitionB = FGWBWorkGroupDefinition();
			WorkGroupDefinitionB.Id = WorkGroupB;
			WorkGroupDefinitionB.Priority = 1;
			WorkGroupDefinitionB.MaxFrameBudget = 0.1;
			Manager->WorkGroups.Add(FGWBWorkGroup(WorkGroupDefinitionB));
		});
		AfterEach([this]()
		{
			if (Manager->IsValidLowLevel()) Manager->ConditionalBeginDestroy();
		});
		
		It("should stop doing work in a group when the group budget is exhausted", [this]()
		{
			const FName WorkGroupA = FName("WorkGroupA");
			const FName WorkGroupB = FName("WorkGroupB");
			FScopedCVarOverrideFloat CvarFrameBudget(TEXT("gwb.budget.frame"), 0.5f);
			Manager->ScheduleWork( WorkGroupA, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.1); });
			Manager->ScheduleWork( WorkGroupA, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.1); });
			Manager->ScheduleWork( WorkGroupA, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.1); });
			Manager->ScheduleWork( WorkGroupB, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.1); });
			Manager->ScheduleWork( WorkGroupB, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.1); });
			Manager->ScheduleWork( WorkGroupB, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.1); });
			TestTrue("# of scheduled work units is 6", Manager->TEST_GetWorkUnitCount() == 6);
			Manager->DoWork();
			TestTrue("# of scheduled work units is 4", Manager->TEST_GetWorkUnitCount() == 4);
			TestTrue("work group A did one unit of work", Manager->WorkGroups.Find(WorkGroupA)->WorkUnitsQueue.Num() == 2);
			TestTrue("work group B did one unit of work", Manager->WorkGroups.Find(WorkGroupB)->WorkUnitsQueue.Num() == 2);
			Manager->DoWork();
			TestTrue("# of scheduled work units is 2", Manager->TEST_GetWorkUnitCount() == 2);
			TestTrue("work group A did one unit of work", Manager->WorkGroups.Find(WorkGroupA)->WorkUnitsQueue.Num() == 1);
			TestTrue("work group B did one unit of work", Manager->WorkGroups.Find(WorkGroupB)->WorkUnitsQueue.Num() == 1);
		});
		
		It("should stop doing work in a group when max unit count is reached", [this]()
		{
			const FName WorkGroupA = FName("WorkGroupA");
			const FName WorkGroupB = FName("WorkGroupB");
			FScopedCVarOverrideFloat CvarFrameBudget(TEXT("gwb.budget.frame"), 0.5f);
			Manager->ScheduleWork( WorkGroupA, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.f); });
			Manager->ScheduleWork( WorkGroupA, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.f); });
			Manager->ScheduleWork( WorkGroupA, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.f); });
			Manager->ScheduleWork( WorkGroupA, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.f); });
			Manager->ScheduleWork( WorkGroupA, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.f); });
			Manager->ScheduleWork( WorkGroupA, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.f); });
			Manager->ScheduleWork( WorkGroupB, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.f); });
			Manager->ScheduleWork( WorkGroupB, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.f); });
			Manager->ScheduleWork( WorkGroupB, FGWBWorkOptions::EmptyOptions).OnHandleWork([](const float DeltaTime, const FGWBWorkUnitHandle& Handle){ FPlatformProcess::Sleep(0.f); });
			TestTrue("# of scheduled work units is 9", Manager->TEST_GetWorkUnitCount() == 9);
			Manager->DoWork();
			TestTrue("# of scheduled work units is 3", Manager->TEST_GetWorkUnitCount() == 3);
			TestTrue("work group A did one unit of work", Manager->WorkGroups.Find(WorkGroupA)->WorkUnitsQueue.Num() == 3);
			TestTrue("work group B did 3 units of work", Manager->WorkGroups.Find(WorkGroupB)->WorkUnitsQueue.Num() == 0);
			Manager->DoWork();
			TestTrue("# of scheduled work units is 0", Manager->TEST_GetWorkUnitCount() == 0);
		});
	});
	
	Describe("AbortWorkUnit()", [this]()
	{
		PrepareTests();
		It("should NOT perform a unit of work if it's been aborted", [this]()
		{
			bool bCallbackFired = false;
			auto Handle = Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Handle.OnHandleWork([&bCallbackFired](const float DeltaTime, const FGWBWorkUnitHandle& Handle)
			{
				bCallbackFired = true;
			});
			Manager->AbortWorkUnit(Manager,Handle);
			TestTrue("# of scheduled work units is 1", Manager->TEST_GetWorkUnitCount() == 1); // note that aborting a unit does NOT de-schedule it
			Manager->DoWork();
			TestTrue("# of scheduled work units is 0", Manager->TEST_GetWorkUnitCount() == 0);
			TestFalse("Callback should NOT be fired", bCallbackFired == false);
		});
	});
}
UE_ENABLE_OPTIMIZATION

#endif // WITH_DEV_AUTOMATION_TESTS