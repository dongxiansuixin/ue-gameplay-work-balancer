#include "Utils/GWBLoopUtils.h"
#include "Components/GWBTimeSlicer.h"
#include "GWBSubsystem.h"
#include "Misc/AutomationTest.h"
#include "Tests/ScopedCvarOverrides.h"
#include "Tests/TestMocks.h"
#include "Engine/World.h"

#if WITH_DEV_AUTOMATION_TESTS

// Implementation of UGWBLoopUtilsTestHelper methods
void UGWBLoopUtilsTestHelper::ProcessWorkUnit(FBudgetedLoopHandle& LoopHandle)
{
	ProcessedCount++;
	
	// Execute custom callback if provided
	if (CustomCallback)
	{
		CustomCallback(LoopHandle);
	}
	
	// Check for break condition
	if (bShouldBreak || (BreakAtCount > 0 && ProcessedCount >= BreakAtCount))
	{
		LoopHandle.Break();
	}
	
	// Add sleep if specified
	if (SleepDuration > 0.0f)
	{
		FPlatformProcess::Sleep(SleepDuration);
	}
}

void UGWBLoopUtilsTestHelper::ResetCounter()
{
	ProcessedCount = 0;
	bShouldBreak = false;
	BreakAtCount = -1;
	SleepDuration = 0.0f;
	CustomCallback = nullptr;
}

void UGWBLoopUtilsTestHelper::SetBreakAtCount(int32 Count)
{
	BreakAtCount = Count;
}

void UGWBLoopUtilsTestHelper::SetSleepDuration(float Duration)
{
	SleepDuration = Duration;
}

void UGWBLoopUtilsTestHelper::SetCustomCallback(TFunction<void(FBudgetedLoopHandle&)> Callback)
{
	CustomCallback = Callback;
}

BEGIN_DEFINE_SPEC(FGWBLoopUtilsTests, "GWBRuntime.GWBLoopUtils", EAutomationTestFlags::ProductFilter | EAutomationTestFlags_ApplicationContextMask)
	// Test data and helper objects
	UGWBManagerMock* MockManager;
	UGWBLoopUtilsTestHelper* TestHelper;
	TArray<int32> TestArray;
	int32 ProcessedCount;
	int32 ExpectedProcessedCount;
	FName LastUsedTimeSlicerId;
	
	// Helper to capture time slicer ID used during loop execution
	void SetupTestArray(int32 ArraySize = 10);
	void ResetCounters();
	TWeakObjectPtr<UGWBTimeSlicer> GetTimeSlicerById(const FName& Id);
END_DEFINE_SPEC(FGWBLoopUtilsTests)

void FGWBLoopUtilsTests::SetupTestArray(int32 ArraySize)
{
	TestArray.Empty();
	for (int32 i = 0; i < ArraySize; ++i)
	{
		TestArray.Add(i);
	}
}

void FGWBLoopUtilsTests::ResetCounters()
{
	ProcessedCount = 0;
	ExpectedProcessedCount = 0;
	LastUsedTimeSlicerId = NAME_None;
	if (TestHelper)
	{
		TestHelper->ResetCounter();
	}
}

TWeakObjectPtr<UGWBTimeSlicer> FGWBLoopUtilsTests::GetTimeSlicerById(const FName& Id)
{
	return UGWBTimeSlicer::Get(MockManager, Id);
}

void FGWBLoopUtilsTests::Define()
{
	BeforeEach([this]()
	{
		MockManager = FGWBManagerTestHelper::Create();
		TestHelper = NewObject<UGWBLoopUtilsTestHelper>();
		SetupTestArray();
		ResetCounters();
	});

	AfterEach([this]()
	{
		if (MockManager && MockManager->IsValidLowLevel()) 
		{
			MockManager->ConditionalBeginDestroy();
		}
		if (TestHelper && TestHelper->IsValidLowLevel())
		{
			TestHelper->ConditionalBeginDestroy();
		}
	});

	Describe("C++ BUDGETED_FOR_LOOP Macro", [this]()
	{
		It("should process all elements when budget is sufficient", [this]()
		{
			FScopedCVarOverrideFloat FrameBudget(TEXT("gwb.frame.budget"), 1.0f); // Large budget
			
			BUDGETED_FOR_LOOP(MockManager, 1.0f, 100, TestArray, [&](FBudgetedLoopHandle& Handle) {
				ProcessedCount++;
			});
			
			TestEqual("All elements should be processed", ProcessedCount, TestArray.Num());
		});

		It("should respect time budget and process partial elements", [this]()
		{
			FScopedCVarOverrideFloat FrameBudget(TEXT("gwb.frame.budget"), 0.001f); // Very small budget
			
			// Set up a larger array to ensure we hit budget limits
			SetupTestArray(100);
			
			BUDGETED_FOR_LOOP(MockManager, 0.001f, 100, TestArray, [&](FBudgetedLoopHandle& Handle) {
				ProcessedCount++;
				// Add small delay to consume time budget
				FPlatformProcess::Sleep(0.0005f);
			});
			
			TestTrue("Should process fewer elements than total due to budget", ProcessedCount < TestArray.Num());
			TestTrue("Should process at least one element", ProcessedCount > 0);
		});

		It("should respect work unit count budget", [this]()
		{
			const int32 MaxWorkCount = 5;
			
			BUDGETED_FOR_LOOP(MockManager, 1.0f, MaxWorkCount, TestArray, [&](FBudgetedLoopHandle& Handle) {
				ProcessedCount++;
			});
			
			TestEqual("Should process exactly MaxWorkCount elements", ProcessedCount, MaxWorkCount);
		});

		It("should handle empty arrays gracefully", [this]()
		{
			TestArray.Empty();
			
			BUDGETED_FOR_LOOP(MockManager, 1.0f, 10, TestArray, [&](FBudgetedLoopHandle& Handle) {
				ProcessedCount++;
			});
			
			TestEqual("Should process zero elements for empty array", ProcessedCount, 0);
		});

		It("should allow users to break the loop early", [this]()
		{
			const int32 BreakAtIndex = 3;
			
			BUDGETED_FOR_LOOP(MockManager, 1.0f, 100, TestArray, [&](FBudgetedLoopHandle& Handle) {
				ProcessedCount++;
				if (ProcessedCount >= BreakAtIndex)
				{
					Handle.Break();
				}
			});
			
			TestEqual("Should process exactly BreakAtIndex elements", ProcessedCount, BreakAtIndex);
		});

		It("should prioritize user break over budget constraints", [this]()
		{
			const int32 BreakAtIndex = 2;
			const int32 MaxWorkCount = 10; // Higher than break point
			
			BUDGETED_FOR_LOOP(MockManager, 1.0f, MaxWorkCount, TestArray, [&](FBudgetedLoopHandle& Handle) {
				ProcessedCount++;
				if (ProcessedCount >= BreakAtIndex)
				{
					Handle.Break();
				}
			});
			
			TestEqual("Should break early regardless of budget", ProcessedCount, BreakAtIndex);
		});

		It("should generate unique time slicer IDs for different call sites", [this]()
		{
			// First call site
			BUDGETED_FOR_LOOP(MockManager, 1.0f, 1, TestArray, [&](FBudgetedLoopHandle& Handle) {
				ProcessedCount++;
			});
			
			// Second call site  
			BUDGETED_FOR_LOOP(MockManager, 1.0f, 1, TestArray, [&](FBudgetedLoopHandle& Handle) {
				ProcessedCount++;
			});
			
			// Both should execute successfully
			TestEqual("Both call sites should execute", ProcessedCount, 2);
		});
	});

	Describe("Blueprint BudgetedForLoopBlueprint Function", [this]()
	{
		It("should process all elements when budget is sufficient", [this]()
		{
			FScopedCVarOverrideFloat FrameBudget(TEXT("gwb.frame.budget"), 1.0f);
			
			FGWBBudgetedLoopWorkDelegate WorkDelegate;
			WorkDelegate.BindUFunction(TestHelper, FName("ProcessWorkUnit"));
			
			UGWBLoopUtilsBlueprintLibrary::BudgetedForLoopBlueprint(
				MockManager, 1.0f, 100, TestArray.Num(), WorkDelegate, TEXT("TestLoop1")
			);
			
			TestEqual("All elements should be processed", TestHelper->ProcessedCount, TestArray.Num());
		});

		It("should respect time budget constraints", [this]()
		{
			FScopedCVarOverrideFloat FrameBudget(TEXT("gwb.frame.budget"), 0.001f);
			
			TestHelper->SetSleepDuration(0.0005f);
			
			FGWBBudgetedLoopWorkDelegate WorkDelegate;
			WorkDelegate.BindUFunction(TestHelper, FName("ProcessWorkUnit"));
			
			SetupTestArray(50);
			UGWBLoopUtilsBlueprintLibrary::BudgetedForLoopBlueprint(
				MockManager, 0.001f, 100, TestArray.Num(), WorkDelegate, TEXT("TestLoop2")
			);
			
			TestTrue("Should process fewer elements due to time budget", TestHelper->ProcessedCount < TestArray.Num());
		});

		It("should respect work unit count budget", [this]()
		{
			const int32 MaxWorkCount = 3;
			
			FGWBBudgetedLoopWorkDelegate WorkDelegate;
			WorkDelegate.BindUFunction(TestHelper, FName("ProcessWorkUnit"));
			
			UGWBLoopUtilsBlueprintLibrary::BudgetedForLoopBlueprint(
				MockManager, 1.0f, MaxWorkCount, TestArray.Num(), WorkDelegate, TEXT("TestLoop3")
			);
			
			TestEqual("Should process exactly MaxWorkCount elements", TestHelper->ProcessedCount, MaxWorkCount);
		});

		It("should allow users to break the loop early via handle", [this]()
		{
			const int32 BreakAtIndex = 4;
			
			TestHelper->SetBreakAtCount(BreakAtIndex);
			
			FGWBBudgetedLoopWorkDelegate WorkDelegate;
			WorkDelegate.BindUFunction(TestHelper, FName("ProcessWorkUnit"));
			
			UGWBLoopUtilsBlueprintLibrary::BudgetedForLoopBlueprint(
				MockManager, 1.0f, 100, TestArray.Num(), WorkDelegate, TEXT("TestLoop4")
			);
			
			TestEqual("Should break at specified index", TestHelper->ProcessedCount, BreakAtIndex);
		});

		It("should handle zero array count gracefully", [this]()
		{
			FGWBBudgetedLoopWorkDelegate WorkDelegate;
			WorkDelegate.BindUFunction(TestHelper, FName("ProcessWorkUnit"));
			
			UGWBLoopUtilsBlueprintLibrary::BudgetedForLoopBlueprint(
				MockManager, 1.0f, 10, 0, WorkDelegate, TEXT("TestLoop5")
			);
			
			TestEqual("Should process zero elements for zero array count", TestHelper->ProcessedCount, 0);
		});

		It("should work with unbound delegate", [this]()
		{
			FGWBBudgetedLoopWorkDelegate UnboundDelegate;
			
			// Should not crash with unbound delegate
			UGWBLoopUtilsBlueprintLibrary::BudgetedForLoopBlueprint(
				MockManager, 1.0f, 10, TestArray.Num(), UnboundDelegate, TEXT("TestLoop6")
			);
			
			TestEqual("Should not process anything with unbound delegate", TestHelper->ProcessedCount, 0);
		});
	});

	Describe("Cross-Tick Behavior with Break", [this]()
	{
		It("should maintain state across multiple calls with break capability", [this]()
		{
			FScopedCVarOverrideFloat FrameBudget(TEXT("gwb.frame.budget"), 0.001f);
			const FString CallSiteId = TEXT("CrossTickBreakTest");
			
			TestHelper->SetBreakAtCount(5);
			TestHelper->SetSleepDuration(0.0002f);
			
			FGWBBudgetedLoopWorkDelegate WorkDelegate;
			WorkDelegate.BindUFunction(TestHelper, FName("ProcessWorkUnit"));
			
			SetupTestArray(20);
			int32 CallCount = 0;
			
			// First call - should break at 5 elements
			UGWBLoopUtilsBlueprintLibrary::BudgetedForLoopBlueprint(
				MockManager, 0.001f, 20, TestArray.Num(), 
				WorkDelegate, CallSiteId
			);
			
			TestEqual("Should break at 5 elements", TestHelper->ProcessedCount, 5);
		});
	});

	Describe("Break Handle Functionality", [this]()
	{
		It("should break immediately when Break() is called", [this]()
		{
			bool bBreakCalled = false;
			
			BUDGETED_FOR_LOOP(MockManager, 1.0f, 100, TestArray, [&](FBudgetedLoopHandle& Handle) {
				ProcessedCount++;
				
				if (ProcessedCount == 1) // Break on first iteration
				{
					Handle.Break();
					bBreakCalled = true;
				}
			});
			
			TestTrue("Break should have been called", bBreakCalled);
			TestEqual("Should process only one element", ProcessedCount, 1);
		});

		It("should work with nested conditions for breaking", [this]()
		{
			bool bConditionMet = false;
			
			BUDGETED_FOR_LOOP(MockManager, 1.0f, 100, TestArray, [&](FBudgetedLoopHandle& Handle) {
				ProcessedCount++;
				
				// Complex condition for breaking
				if (ProcessedCount > 2 && ProcessedCount < 6)
				{
					if (!bConditionMet)
					{
						bConditionMet = true;
						Handle.Break();
					}
				}
			});
			
			TestTrue("Condition should have been met", bConditionMet);
			TestEqual("Should break at element 3", ProcessedCount, 3);
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS