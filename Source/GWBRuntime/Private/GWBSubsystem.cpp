// Fill out your copyright notice in the Description page of Project Settings.


#include "GWBSubsystem.h"
#include "GWBManager.h"

void UGWBSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Manager = NewObject<UGWBManager>();
	
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UGWBSubsystem::OnPostWorldInitialization);
	FWorldDelegates::OnWorldBeginTearDown.AddUObject(this, &UGWBSubsystem::OnWorldBeginTearDown);
}

void UGWBSubsystem::Deinitialize()
{
	Super::Deinitialize();
	Manager->Reset();
}

UGWBManager* UGWBSubsystem::GetManager() const
{
	return Manager;
}

void UGWBSubsystem::OnPostWorldInitialization(UWorld* World, const UWorld::InitializationValues IVS)
{
	if (World->IsGameWorld())
	{
		Manager->Initialize(World);
	}
}

void UGWBSubsystem::OnWorldBeginTearDown(UWorld* World)
{
	if (World->IsGameWorld())
	{
		Manager->Reset();
	}
}
