// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/SoloPlayerController.h"
#include "TopDownPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SOLO_API ATopDownPlayerController : public ASoloPlayerController
{
	GENERATED_BODY()
public:
	ATopDownPlayerController();

	virtual void PostProcessInput(float DeltaTime, bool bGamePaused) override;

	UPROPERTY(EditDefaultsOnly)
	float GamepadCursorOffsetMultiplier = 200.f;

	FVector2D CursorOffset;
};
