// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "OnlineSessionClient.h"
#include "SoloOnlineSession.generated.h"

/**
 * 
 */
UCLASS()
class SOLO_API USoloOnlineSession : public UOnlineSessionClient
{
	GENERATED_BODY()
public:
	USoloOnlineSession();
};
