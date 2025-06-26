// Fill out your copyright notice in the Description page of Project Settings.


#include "GWBSubsystem.h"
#include "GWBManager.h"

void UGWBSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	Manager = NewObject<UGWBManager>();
}

void UGWBSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

UGWBManager* UGWBSubsystem::GetManager() const
{
	return Manager;
}

