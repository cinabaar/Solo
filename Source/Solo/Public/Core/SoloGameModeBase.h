// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SoloGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class SOLO_API ASoloGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public:
	ASoloGameModeBase();
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	virtual void StartPlay() override;
	virtual void GetSeamlessTravelActorList(bool bToTransition, TArray<AActor*>& ActorList) override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void PostSeamlessTravel() override;
	virtual void StartToLeaveMap() override;
	virtual void PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void Logout(AController* Exiting) override;
	virtual void RestartPlayerAtPlayerStart(AController* NewPlayer, AActor* StartSpot) override;
	virtual void RestartPlayerAtTransform(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual void SetPlayerDefaults(APawn* PlayerPawn) override;
	virtual bool AllowCheats(APlayerController* P) override;
	virtual void GenericPlayerInitialization(AController* C) override;
	virtual FString InitNewPlayer(APlayerController* NewPlayerController, const FUniqueNetIdRepl& UniqueId, const FString& Options, const FString& Portal = TEXT("")) override;
	virtual void InitSeamlessTravelPlayer(AController* NewController) override;

	virtual UClass* GetDefaultPawnClassForController_Implementation(AController* InController) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual AActor* FindPlayerStart_Implementation(AController* Player, const FString& IncomingName = TEXT("")) override;
	virtual APawn* SpawnDefaultPawnFor_Implementation(AController* NewPlayer, AActor* StartSpot) override;
	virtual APawn* SpawnDefaultPawnAtTransform_Implementation(AController* NewPlayer, const FTransform& SpawnTransform) override;
	virtual void InitStartSpot_Implementation(AActor* StartSpot, AController* NewPlayer) override;
	virtual bool PlayerCanRestart_Implementation(APlayerController* Player) override;
};
