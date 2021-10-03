// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/SoloCameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"

ASoloCameraActor::ASoloCameraActor()
{
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	// Make the scene component the root component
	RootComponent = SceneComponent;

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	// Setup camera defaults
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->FieldOfView = 90.0f;
	CameraComponent->bConstrainAspectRatio = true;
	CameraComponent->AspectRatio = 1.777778f;
	CameraComponent->PostProcessBlendWeight = 1.0f;

	CameraComponent->SetupAttachment(SpringArm);

	PrimaryActorTick.bCanEverTick = true;
}


void ASoloCameraActor::ControlledPawnChanged(APawn* NewPawn)
{
	CachedPawn = NewPawn;
}

void ASoloCameraActor::BeginPlay()
{
	Super::BeginPlay();
	SetActorRotation(CameraWorldRotation);
	if(PlayerController && PlayerController->GetPawn())
	{
		ControlledPawnChanged(PlayerController->GetPawn());
	}
}

void ASoloCameraActor::InitializeFor(APlayerController* PC)
{
	PlayerController = PC;
}

void ASoloCameraActor::OnConstruction(const FTransform& Transform)
{
	SpringArm->SetRelativeRotation(SpringArmRotation);
	SpringArm->TargetArmLength = TargetArmLength;
}

void ASoloCameraActor::Tick(float DeltaSeconds)
{
	if(PlayerController && CachedPawn != PlayerController->GetPawn())
	{
		ControlledPawnChanged(PlayerController->GetPawn());
		return;
	}
	
	if(CachedPawn)
	{
		SetActorLocation(CachedPawn->GetActorLocation());
	}
}

