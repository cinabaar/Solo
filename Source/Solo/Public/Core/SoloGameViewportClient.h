// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameViewportClient.h"
#include "SoloGameViewportClient.generated.h"

/**
 * 
 */
UCLASS()
class SOLO_API USoloGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:
	USoloGameViewportClient();
};
