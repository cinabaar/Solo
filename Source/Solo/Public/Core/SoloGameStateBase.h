// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "SoloGameStateBase.generated.h"

UCLASS()
class SOLO_API ASoloGameStateBase : public AGameStateBase
{
	GENERATED_BODY()
public:
	ASoloGameStateBase();
	virtual void BeginPlay() override;
	virtual float GetPlayerRespawnDelay(AController* Controller) const override;
	virtual void AddPlayerState(APlayerState* PlayerState) override;
	virtual void RemovePlayerState(APlayerState* PlayerState) override;
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > &OutLifetimeProps) const override;
};
