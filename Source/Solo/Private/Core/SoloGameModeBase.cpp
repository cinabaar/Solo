// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.


#include "Core/SoloGameModeBase.h"
#include "Character/SoloCharacter.h"
#include "Character/SoloSpectatorPawn.h"
#include "Core/SoloGameStateBase.h"
#include "HUD/SoloHUD.h"
#include "Online/SoloGameSession.h"
#include "Player/SoloPlayerController.h"
#include "Player/SoloPlayerState.h"
#include "Engine/World.h"

ASoloGameModeBase::ASoloGameModeBase() 
{
	GameSessionClass = ASoloGameSession::StaticClass();
	GameStateClass = ASoloGameStateBase::StaticClass();
	PlayerControllerClass = ASoloPlayerController::StaticClass();
	PlayerStateClass = ASoloPlayerState::StaticClass();
	HUDClass = ASoloHUD::StaticClass();
	DefaultPawnClass = ASoloCharacter::StaticClass();
	SpectatorClass = ASoloSpectatorPawn::StaticClass();
	ReplaySpectatorPlayerControllerClass = APlayerController::StaticClass();
	ServerStatReplicatorClass = AServerStatReplicator::StaticClass();

	bPauseable = true;
	bStartPlayersAsSpectators = false;
	bUseSeamlessTravel = !GIsEditor;
}

void ASoloGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
}

void ASoloGameModeBase::InitGameState()
{
	Super::InitGameState();
}

void ASoloGameModeBase::StartPlay()
{
	Super::StartPlay();
}

void ASoloGameModeBase::GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToTransition, ActorList);
}

void ASoloGameModeBase::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);
}

void ASoloGameModeBase::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
}

void ASoloGameModeBase::StartToLeaveMap()
{
	Super::StartToLeaveMap();
}

void ASoloGameModeBase::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);
}

APlayerController* ASoloGameModeBase::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);
}

void ASoloGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);
}

void ASoloGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);
}

void ASoloGameModeBase::RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot)
{
	Super::RestartPlayerAtPlayerStart(NewPlayer, StartSpot);
}

void ASoloGameModeBase::RestartPlayerAtTransform(AController* NewPlayer, const FTransform& SpawnTransform)
{
	Super::RestartPlayerAtTransform(NewPlayer, SpawnTransform);
}

void ASoloGameModeBase::SetPlayerDefaults(APawn* PlayerPawn)
{
	Super::SetPlayerDefaults(PlayerPawn);
}

bool ASoloGameModeBase::AllowCheats(APlayerController* P)
{
	return Super::AllowCheats(P);
}

void ASoloGameModeBase::GenericPlayerInitialization(AController* C)
{
	Super::GenericPlayerInitialization(C);
}

FString ASoloGameModeBase::InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal)
{
	return Super::InitNewPlayer(NewPlayerController, UniqueId, Options, Portal);
}

void ASoloGameModeBase::InitSeamlessTravelPlayer(AController* NewController)
{
	Super::InitSeamlessTravelPlayer(NewController);
}

UClass* ASoloGameModeBase::GetDefaultPawnClassForController_Implementation(AController* InController)
{
	return Super::GetDefaultPawnClassForController_Implementation(InController);
}

void ASoloGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);
}

AActor* ASoloGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	return Super::ChoosePlayerStart_Implementation(Player);
}

AActor* ASoloGameModeBase::FindPlayerStart_Implementation(AController* Player, const FString& IncomingName)
{
	return Super::FindPlayerStart_Implementation(Player, IncomingName);
}

APawn* ASoloGameModeBase::SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot)
{
	return Super::SpawnDefaultPawnFor_Implementation(NewPlayer, StartSpot);
}

APawn* ASoloGameModeBase::SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform)
{
	return Super::SpawnDefaultPawnAtTransform_Implementation(NewPlayer, SpawnTransform);
}

void ASoloGameModeBase::InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer)
{
	Super::InitStartSpot_Implementation(StartSpot, NewPlayer);
}

bool ASoloGameModeBase::PlayerCanRestart_Implementation(APlayerController* Player)
{
	return Super::PlayerCanRestart_Implementation(Player);
}
