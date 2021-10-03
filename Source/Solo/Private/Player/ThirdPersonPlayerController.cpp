// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/ThirdPersonPlayerController.h"


AThirdPersonPlayerController::AThirdPersonPlayerController()
{
}

void AThirdPersonPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);
	return;

	if (!GetPawn())
	{
		return;
	}

	FVector MoveVector = GetPawn()->GetActorRotation().RotateVector(FVector{ MoveInput, 0.f });
	GetPawn()->AddMovementInput(MoveVector, 1.f);

	AddYawInput(MouseInput.X);
	AddPitchInput(MouseInput.Y);

	FVector EyeLocation;
	FRotator EyeRotation;
	GetPawn()->GetActorEyesViewPoint(EyeLocation, EyeRotation);
	FVector LookPoint = EyeLocation + EyeRotation.Vector() * 1000.f;
	UpdateCursorVisualization(LookPoint);
}
