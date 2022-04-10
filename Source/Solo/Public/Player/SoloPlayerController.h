// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "SoloPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SOLO_API ASoloPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ASoloPlayerController();
	
	virtual void GetSeamlessTravelActorList(bool bToEntry, TArray<class AActor*>& ActorList) override;
	virtual void SeamlessTravelTo(class APlayerController* NewPC) override;
	virtual void SeamlessTravelFrom(class APlayerController* OldPC) override;
	virtual void PostSeamlessTravel() override;
	virtual void PostProcessInput(float DeltaTime, bool bGamePaused) override;

	virtual void InitInputSystem() override;
	virtual void SetPawn(APawn* InPawn) override;
	UFUNCTION(BlueprintImplementableEvent)
	void OnPawnSet(APawn* InPawn);
protected:
	
	UFUNCTION(BlueprintImplementableEvent, BlueprintCosmetic)
	void UpdateCursorVisualization(FVector WorldCursorLocation);

	virtual void SetupInputComponent() override;
	FVector2D MoveInput;
	FVector2D LookInput;

	FVector2D MouseInput;
	bool bAiming = false;

	float DebugPitch;

};
