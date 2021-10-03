// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "SoloCharacterMovementComponent.generated.h"


class FSavedMove_SoloCharacter : public FSavedMove_Character
{
public:
	using Super = FSavedMove_Character;
};

class FNetworkPredictionData_Client_SoloCharacter : public FNetworkPredictionData_Client_Character
{
public:
	FNetworkPredictionData_Client_SoloCharacter(const UCharacterMovementComponent& ClientMovement);

	using Super = FNetworkPredictionData_Client_Character;

	virtual FSavedMovePtr AllocateNewMove() override;
};

UCLASS()
class SOLO_API USoloCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	USoloCharacterMovementComponent();
	
	FVector PrevLocation;
	FVector PrevVelocity;

	virtual void UpdateProxyAcceleration() override;
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void PerformMovement(float DeltaTime) override;
	UFUNCTION(BlueprintCallable)
	FVector PredictStopLocation();
};
