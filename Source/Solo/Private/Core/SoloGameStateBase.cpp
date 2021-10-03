// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/SoloGameStateBase.h"
#include "Net/UnrealNetwork.h"



ASoloGameStateBase::ASoloGameStateBase()
{
}

void ASoloGameStateBase::BeginPlay()
{
	Super::BeginPlay();
}

float ASoloGameStateBase::GetPlayerRespawnDelay(AController* Controller) const
{
	return Super::GetPlayerRespawnDelay(Controller);
}

void ASoloGameStateBase::AddPlayerState(APlayerState* PlayerState)
{
	Super::AddPlayerState(PlayerState);
}

void ASoloGameStateBase::RemovePlayerState(APlayerState* PlayerState)
{
	Super::RemovePlayerState(PlayerState);
}

void ASoloGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}