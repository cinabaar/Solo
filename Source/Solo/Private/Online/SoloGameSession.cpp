// Fill out your copyright notice in the Description page of Project Settings.


#include "Online/SoloGameSession.h"

#include "Kismet/GameplayStatics.h"
#include "OnlineSubsystemUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogSoloGameSession, All, Log)

ASoloGameSession::ASoloGameSession()
{
	PrimaryActorTick.bCanEverTick = true;
	SessionName = NAME_GameSession;
}

void ASoloGameSession::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ASoloGameSession::RegisterServer()
{
	Super::RegisterServer();
}

void ASoloGameSession::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

