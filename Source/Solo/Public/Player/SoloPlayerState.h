// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "SaveGame/SoloSaveGame.h"
#include "AbilitySystemInterface.h"
#include "SoloPlayerState.generated.h"

class USoloAbilitySystemComponent;
UCLASS()
class SOLO_API ASoloPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()
	DECLARE_EVENT_OneParam(ASoloPlayerState, FSoloPlayerStateEvent, ASoloPlayerState*)
public:
	ASoloPlayerState();
	virtual void ClientInitialize(class AController* C) override;
	virtual void RegisterPlayerWithSession(bool bWasFromInvite) override;
	virtual void UnregisterPlayerWithSession() override;
	virtual void SeamlessTravelTo(class APlayerState* NewPlayerState) override;
	//BEGIN IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//END IAbilitySystemInterface
protected:
	virtual void CopyProperties(APlayerState* PlayerState) override;
	UPROPERTY()
	USoloAbilitySystemComponent* AbilitySystemComponent = nullptr;
};
