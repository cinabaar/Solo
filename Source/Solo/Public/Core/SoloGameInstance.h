// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "SoloGameInstance.generated.h"


UCLASS()
class SOLO_API USoloGameInstance : public UGameInstance
{	
	GENERATED_BODY()
public:
	USoloGameInstance();
	virtual void Init() override;
	virtual void Shutdown() override;
	virtual void OnWorldChanged(UWorld* OldWorld, UWorld* NewWorld) override;
	virtual void StartGameInstance() override;
	virtual ULocalPlayer* CreateInitialPlayer(FString& OutError) override;
	virtual TSubclassOf<UOnlineSession> GetOnlineSessionClass() override;

protected:
	virtual void OnStart() override;
};
