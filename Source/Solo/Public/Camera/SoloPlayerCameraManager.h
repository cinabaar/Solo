// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "SoloPlayerCameraManager.generated.h"

class ASoloCameraActor;
/**
 * 
 */
UCLASS()
class SOLO_API ASoloPlayerCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditDefaultsOnly, Category = "Camera")
	TSubclassOf<ASoloCameraActor> CameraActorClass;
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	ASoloCameraActor* CameraActor = nullptr;
	
	virtual void InitializeFor(class APlayerController* PC) override;
};
