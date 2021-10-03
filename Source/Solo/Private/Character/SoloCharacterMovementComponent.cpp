// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SoloCharacterMovementComponent.h"

#include "Character/SoloCharacter.h"
#include "Net/UnrealNetwork.h"

FNetworkPredictionData_Client_SoloCharacter::FNetworkPredictionData_Client_SoloCharacter(const UCharacterMovementComponent& ClientMovement) : Super(ClientMovement)
{
}

FSavedMovePtr FNetworkPredictionData_Client_SoloCharacter::AllocateNewMove()
{
	return MakeShared<FSavedMove_SoloCharacter>();
}

USoloCharacterMovementComponent::USoloCharacterMovementComponent()
{
}

void USoloCharacterMovementComponent::UpdateProxyAcceleration()
{
	auto* SoloCharacter = Cast<ASoloCharacter>(GetCharacterOwner());
	if(!SoloCharacter->IsAccelerating()) //attempt to detect the case when we are stopping and set acceleration to 0
	{
		Acceleration = FVector::ZeroVector;
		AnalogInputModifier = 1.0f;
	}
	else
	{
		Super::UpdateProxyAcceleration();
	}
}

void USoloCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	PrevLocation = OldLocation;
	PrevVelocity = OldVelocity;
}

void USoloCharacterMovementComponent::PerformMovement(float DeltaTime)
{
	auto* SoloCharacter = Cast<ASoloCharacter>(GetCharacterOwner());
	bool bAccelerating = !Acceleration.IsZero();
	SoloCharacter->SetAccelerating(bAccelerating);
	Super::PerformMovement(DeltaTime);
}

FVector USoloCharacterMovementComponent::PredictStopLocation()
{
	auto CachedVelocity = Velocity;
	auto TempPrevVelocity = FVector::ZeroVector;
	float Friction = (bUseSeparateBrakingFriction ? BrakingFriction : GroundFriction);
	float DeltaTime = GetWorld()->GetDeltaSeconds();
	FVector OutLocation = LastUpdateLocation;
	FVector Delta = Velocity * DeltaTime;
	
	while (!Velocity.IsZero() && TempPrevVelocity != Velocity)
	{
		TempPrevVelocity = Velocity;
		float Iterations = 0;
		float RemainingTime = DeltaTime;
		while ((RemainingTime >= MIN_TICK_TIME) && (Iterations < MaxSimulationIterations))
		{
			Iterations++;
			const float timeTick = GetSimulationTimeStep(RemainingTime, Iterations);
			RemainingTime -= timeTick;

			ApplyVelocityBraking(timeTick, Friction, GetMaxBrakingDeceleration());
			Delta = Velocity * timeTick;
			OutLocation += Delta;
		}
	}
	TempPrevVelocity = Velocity;
	Velocity = CachedVelocity;
	if (TempPrevVelocity.IsZero()) //if this is not true then something went wrong with braking
	{
		return OutLocation;
	}
	return LastUpdateLocation;
}
