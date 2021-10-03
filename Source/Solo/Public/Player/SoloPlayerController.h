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
	void MoveRight(float Val);
	void MoveForward(float Val);
	FVector2D LookInput;
	void TurnRight(float Val);
	void TurnForward(float Val);

	FVector2D MouseInput;
	void MouseX(float Val);
	void MouseY(float Val);
	bool bAiming = false;
	void MouseAimOn(FKey Key);
	void MouseAimOff(FKey Key);
	float DebugPitch;
	void DebugPitchUp(float Val);
	void DebugPitchDown(float Val);
	void JumpPressed(FKey Key);
	void JumpReleased(FKey Key);
};
