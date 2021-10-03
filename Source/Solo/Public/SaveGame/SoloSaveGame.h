// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "SoloSaveGame.generated.h"

class ASoloCharacter;

USTRUCT(BlueprintType)
struct FSoloSaveGameData
{
	GENERATED_BODY()
};

UCLASS()
class SOLO_API USoloSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
	UPROPERTY(SaveGame, EditAnywhere, BlueprintReadOnly, meta = (ShowOnlyInnerProperties))
	FSoloSaveGameData SaveGame;
};
