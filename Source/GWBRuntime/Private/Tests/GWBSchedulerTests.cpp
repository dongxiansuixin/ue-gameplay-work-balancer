#include "Components/GWBScheduler.h"
#include "Misc/AutomationTest.h"
#include "Tests/AutomationCommon.h"

#if WITH_DEV_AUTOMATION_TESTS

struct FGWBSchedulerTestHelper
{
	static UGWBScheduler* CreateScheduler(UObject* Outer = nullptr)
	{
		if (!Outer)
		{
			Outer = GetTransientPackage();
		}
		UGWBScheduler* Scheduler = NewObject<UGWBScheduler>(Outer);
		check(Scheduler);
		return Scheduler;
	}
};

IMPLEMENT_SIMPLE_AUTOMATION_TEST(GWBSchedulerTests, "GWBRuntime.Scheduler.Start", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool GWBSchedulerTests::RunTest(const FString& Parameters)
{
	// Create a scheduler
	UGWBScheduler* Scheduler = FGWBSchedulerTestHelper::CreateScheduler();
    
	// Test starting the scheduler
	bool bDelegateWasCalled = false;
    
	Scheduler->StartWorkCycleDelegate.BindLambda([&bDelegateWasCalled](){ 
		bDelegateWasCalled = true; 
	});
    
	Scheduler->Start();
    
	// Allow the engine to tick once to process any timers
	ADD_LATENT_AUTOMATION_COMMAND(FExecStringLatentCommand(TEXT("AUTOMATION RUNFRAME 1")));
    
	// Check if the delegate was called
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.1f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([bDelegateWasCalled, this]() {
		return TestTrue("StartWorkCycleDelegate should be called after Start()", bDelegateWasCalled);
	}));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FGWBSchedulerStopTest, "GWBRuntime.Scheduler.Stop", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
bool FGWBSchedulerStopTest::RunTest(const FString& Parameters)
{
	// Create a scheduler
	UGWBScheduler* Scheduler = FGWBSchedulerTestHelper::CreateScheduler();
    
	// Set up a counter to track how many times the delegate is called
	int32 CallCount = 0;
	Scheduler->StartWorkCycleDelegate.BindLambda([&CallCount](){ 
		CallCount++; 
	});
    
	// Start and then immediately stop the scheduler
	Scheduler->Start();
	Scheduler->Stop();
    
	// Allow the engine to tick multiple frames to see if any callbacks happen
	ADD_LATENT_AUTOMATION_COMMAND(FExecStringLatentCommand(TEXT("AUTOMATION RUNFRAME 5")));
    
	// Check that after stopping, no further delegate calls occurred
	ADD_LATENT_AUTOMATION_COMMAND(FEngineWaitLatentCommand(0.5f));
	ADD_LATENT_AUTOMATION_COMMAND(FFunctionLatentCommand([CallCount, this]() {
		// If the Stop() worked properly, the call count should be either 0 or 1
		// (0 if Stop() was called before any tick occurred, 1 if a tick happened before Stop())
		return TestTrue("CallCount should be 0 or 1 after Stop()", CallCount <= 1);
	}));
    
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS