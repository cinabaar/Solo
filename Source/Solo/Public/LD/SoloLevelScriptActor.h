// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LevelScriptActor.h"
#include "SoloLevelScriptActor.generated.h"

/**
 * 
 */
UCLASS()
class SOLO_API ASoloLevelScriptActor : public ALevelScriptActor
{
	GENERATED_BODY()
public:
	ASoloLevelScriptActor();
	virtual void BeginPlay() override;
	const TArray<AActor*>& GetInteractables() {return Interactables;}
protected:
	UPROPERTY()
	TArray<AActor*> Interactables;
	FDelegateHandle OnActorSpawnedDelegateHandle;
	UFUNCTION()
	void OnInteractableEndPlay(AActor* Actor , EEndPlayReason::Type EndPlayReason);
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;	
};
