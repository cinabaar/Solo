// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SoloGameInstance.h"
#include "OnlineSubsystemUtils.h"
#include "Online/SoloOnlineSession.h"
#include "Solo/Solo.h"

USoloGameInstance::USoloGameInstance()
{
	
}

void USoloGameInstance::Init()
{
	Super::Init();
}

void USoloGameInstance::Shutdown()
{
	Super::Shutdown();
}

void USoloGameInstance::OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld)
{
	Super::OnWorldChanged(OldWorld, NewWorld);
}

void USoloGameInstance::StartGameInstance()
{
	Super::StartGameInstance();
}

ULocalPlayer* USoloGameInstance::CreateInitialPlayer(FString& OutError)
{
	return Super::CreateInitialPlayer(OutError);
}

TSubclassOf<UOnlineSession> USoloGameInstance::GetOnlineSessionClass()
{
	return USoloOnlineSession::StaticClass();
}

void USoloGameInstance::OnStart()
{
	Super::OnStart();
}