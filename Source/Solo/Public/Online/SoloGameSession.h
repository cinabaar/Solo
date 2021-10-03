// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "SoloGameSession.generated.h"


class FOnlineSessionSearch;

UCLASS()
class SOLO_API ASoloGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	ASoloGameSession();
	virtual void Tick(float DeltaSeconds) override;
	virtual void RegisterServer() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
};
