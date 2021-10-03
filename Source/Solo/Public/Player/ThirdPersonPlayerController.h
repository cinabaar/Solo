// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Player/SoloPlayerController.h"
#include "ThirdPersonPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class SOLO_API AThirdPersonPlayerController : public ASoloPlayerController
{
	GENERATED_BODY()
public:
	AThirdPersonPlayerController();

	virtual void PostProcessInput(float DeltaTime, bool bGamePaused) override;
};
