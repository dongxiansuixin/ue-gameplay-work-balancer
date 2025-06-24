
#include "DataTypes/GWBWorkUnitHandle.h"
#include "GWBRuntimeModule.h"
#include "Misc/AutomationTest.h"
#include "Tests/TestMocks.h"
#include "GWBManager.h"
#include "Components/GWBTimeSlicer.h"
#include "Extensions/Modifiers.h"
#include "Tests/ScopedCvarOverrides.h"

#if WITH_DEV_AUTOMATION_TESTS

BEGIN_DEFINE_SPEC(FGWBExtensionsTests, "GWBRuntime.Extensions", EAutomationTestFlags::ProductFilter | EAutomationTestFlags_ApplicationContextMask)
	// add any member vars here
	UGWBManagerMock* Manager;
	FName WorkGroupID;
	FGWBWorkGroupDefinition WorkGroupDefinition;
	void PrepareTests();
END_DEFINE_SPEC(FGWBExtensionsTests)
void FGWBExtensionsTests::PrepareTests()
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
void FGWBExtensionsTests::Define()
{
	Describe("FFrameBudgetEscalationModifier", [this]()
	{
		PrepareTests();
		It("should NOT modify budget if escalation count is NOT exceeded", [this]()
		{
			FScopedCVarOverrideFloat CvarFrameBudget(TEXT("gwb.frame.budget"), 0.1f);
			const auto Slicer = UGWBTimeSlicer::Get(nullptr, FName("GameplayWorkBalancer"));
			Manager->ModifierManager.AddBudgetModifier(FFrameBudgetEscalationModifier());
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->DoWork();
			TestTrue("frame budget to remain unchanged", Slicer->GetFrameTimeBudget() == 0.1f);
		});
		It("should modify budget if escalation count is exceeded", [this]()
		{
			FScopedCVarOverrideFloat CvarFrameBudget(TEXT("gwb.frame.budget"), 0.1f);
			FScopedCVarOverrideFloat CvarEscalationScalar(TEXT("gwb.escalation.scalar"), 1.0f);
			FScopedCVarOverrideFloat CvarEscalationCount(TEXT("gwb.escalation.count"), 3);
			const auto Slicer = UGWBTimeSlicer::Get(nullptr, FName("GameplayWorkBalancer"));
			Manager->ModifierManager.AddBudgetModifier(FFrameBudgetEscalationModifier());
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->DoWork();
			TestTrue("frame budget has doubled due to escalation", Slicer->GetFrameTimeBudget() == 0.2f);
		});
		It("should decay escalation over time", [this]()
		{
			FScopedCVarOverrideFloat CvarFrameBudget(TEXT("gwb.frame.budget"), 0.1f);
			FScopedCVarOverrideFloat CvarEscalationScalar(TEXT("gwb.escalation.scalar"), 1.0f);
			FScopedCVarOverrideFloat CvarEscalationCount(TEXT("gwb.escalation.count"), 3);
			FScopedCVarOverrideFloat CvarEscalationDecay(TEXT("gwb.escalation.decay"), 0.5);
			const auto Slicer = UGWBTimeSlicer::Get(nullptr, FName("GameplayWorkBalancer"));
			Manager->ModifierManager.AddBudgetModifier(FFrameBudgetEscalationModifier());
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->DoWork();
			TestTrue("frame budget has doubled due to escalation", Slicer->GetFrameTimeBudget() == 0.2f);
			Manager->ScheduleWork( WorkGroupID, { 0, 0, 0, false, false});
			Manager->DoWork();
			TestTrue("frame budget has decayed due to escalation decay", Slicer->GetFrameTimeBudget() < 0.2f);
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS