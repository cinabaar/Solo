// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SoloPlayerController.h"
#include "Debug/SoloCheatManager.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/SoloPlayerCameraManager.h"
#include "Character/SoloCharacter.h"
#include "GameFramework/Character.h"

ASoloPlayerController::ASoloPlayerController()
{
	PlayerCameraManagerClass = ASoloPlayerCameraManager::StaticClass();
	CheatClass = USoloCheatManager::StaticClass();
}

void ASoloPlayerController::GetSeamlessTravelActorList(bool bToEntry, TArray<AActor*>& ActorList)
{
	Super::GetSeamlessTravelActorList(bToEntry, ActorList);
}

void ASoloPlayerController::SeamlessTravelTo(APlayerController* NewPC)
{
	Super::SeamlessTravelTo(NewPC);
}

void ASoloPlayerController::SeamlessTravelFrom(APlayerController* OldPC)
{
	Super::SeamlessTravelFrom(OldPC);
}

void ASoloPlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
}

void ASoloPlayerController::InitInputSystem()
{
	Super::InitInputSystem();
}

void ASoloPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	OnPawnSet(InPawn);
}

void ASoloPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
}

void ASoloPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);
	if(auto* SoloPawn = Cast<ASoloCharacter>(GetPawn()))
	{
		SoloPawn->PostProcessInput(DeltaTime, bGamePaused);
	}
}
