// Fill out your copyright notice in the Description page of Project Settings.


#include "GWBSubsystem.h"
#include "GWBManager.h"

void UGWBSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Manager = NewObject<UGWBManager>();
	Manager->Initialize();
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

