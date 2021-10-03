// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "SoloAbilitySystemComponent.generated.h"

/**
 * 
 */
UCLASS()
class SOLO_API USoloAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
public:
	bool bStartupAbilitiesGiven = false;
	virtual  void AbilityLocalInputPressed(int32 InputID) override;
};
