// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SoloCharacter.h"

#include "Ability/SoloAbilityInputID.h"
#include "Ability/SoloGameplayAbility.h"
#include "Character/SoloCharacterMovementComponent.h"
#include "Components/InputComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Player/SoloPlayerState.h"

// Sets default values
ASoloCharacter::ASoloCharacter(const FObjectInitializer& ObjectInitializer) :
	Super(ObjectInitializer.SetDefaultSubobjectClass<USoloCharacterMovementComponent>(CharacterMovementComponentName))
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

}

// Called when the game starts or when spawned
void ASoloCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void ASoloCharacter::InitAbilitySystemComponent()
{
	ASoloPlayerState* PS = GetPlayerState<ASoloPlayerState>();
	if (PS)
	{
		AbilitySystemComponent = Cast<USoloAbilitySystemComponent>(PS->GetAbilitySystemComponent());
		AbilitySystemComponent->InitAbilityActorInfo(PS, this);
	}
}

void ASoloCharacter::OnRep_ReplicatedAccelerating()
{
}

void ASoloCharacter::BindASCInput()
{
	if (!bASCInputBound && IsValid(AbilitySystemComponent) && IsValid(InputComponent))
	{
		FGameplayAbilityInputBinds InputBinds{ FString("ConfirmTarget"),	FString("CancelTarget"), FString("ESoloAbilityInputID"), static_cast<int32>(ESoloAbilityInputID::Confirm), static_cast<int32>(ESoloAbilityInputID::Cancel) };
		AbilitySystemComponent->BindAbilityActivationToInputComponent(InputComponent, InputBinds);

		bASCInputBound = true;
	}
}

// Called to bind functionality to input
void ASoloCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	// Movement and look input
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis("LookUp", this, &ThisClass::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ThisClass::LookUpRate);
	PlayerInputComponent->BindAxis("Turn", this, &ThisClass::Turn);
	PlayerInputComponent->BindAxis("TurnRate", this, &ThisClass::TurnRate);
	// Bind player input to the AbilitySystemComponent. Also called in OnRep_PlayerState because of a potential race condition.
	BindASCInput();
}

void ASoloCharacter::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	if(!FakeMoveInput.IsZero())
	{
		MoveInput = FakeMoveInput;
	}
	FVector MoveInputWorld = FVector{ MoveInput, 0.f };
	MoveInputWorld = GetActorRotation().RotateVector(MoveInputWorld);
	AddMovementInput(MoveInputWorld, 1.f);

	if (LookRateInput != FVector2D::ZeroVector)
	{
		AddControllerYawInput(LookRateInput.X);
		AddControllerPitchInput(LookRateInput.Y);
	}
	else if(LookInput != FVector2D::ZeroVector)
	{
		AddControllerYawInput(LookInput.X);
		AddControllerPitchInput(LookInput.Y);
	}
	LookRateInput = FVector2D::ZeroVector;
	LookInput = FVector2D::ZeroVector;
	
	//FVector CursorOffset3D = FVector{ CursorOffset, 0.f };
	
	//UpdateCrosshair(GetActorLocation() + CursorOffset3D);
	
	// if (!CursorOffset.IsNearlyZero())
	// {
	// 	FRotator Rot = CursorOffset3D.Rotation();
	// 	GetController()->SetControlRotation(Rot);
	// }
}

void ASoloCharacter::MoveRight(float Val)
{
	MoveInput.Y = Val;
}

void ASoloCharacter::MoveForward(float Val)
{
	MoveInput.X = Val;
}

void ASoloCharacter::LookUp(float Val)
{
	LookInput.Y += Val;
}

void ASoloCharacter::LookUpRate(float Val)
{
	LookRateInput.Y += Val * GetWorld()->GetDeltaSeconds();
}

void ASoloCharacter::Turn(float Val)
{
	LookInput.X = Val;
}

void ASoloCharacter::TurnRate(float Val)
{
	LookRateInput.X = Val * GetWorld()->GetDeltaSeconds();;
}

void ASoloCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
}

void ASoloCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	InitAbilitySystemComponent();
	// Bind player input to the AbilitySystemComponent. Also called in SetupPlayerInputComponent because of a potential race condition.
	BindASCInput();
}

void ASoloCharacter::DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos)
{
	Super::DisplayDebug(Canvas, DebugDisplay, YL, YPos);
}

void ASoloCharacter::GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const
{
	Super::GetActorEyesViewPoint(Location, Rotation);
}

void ASoloCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	InitAbilitySystemComponent();
	if(!AbilitySystemComponent->bStartupAbilitiesGiven)
	{
		for (auto& StartupAbility : StartupAbilities)
		{
			FGameplayAbilitySpec Spec{ StartupAbility, 1, static_cast<int32>(StartupAbility.GetDefaultObject()->AbilityInputID), this };
			AbilitySystemComponent->GiveAbility(Spec);
		}
		AbilitySystemComponent->bStartupAbilitiesGiven = true;
	}
}

void ASoloCharacter::UnPossessed()
{
	Super::UnPossessed();
}

void ASoloCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION(ASoloCharacter, bReplicatedAccelerating, COND_SkipOwner);
}

void ASoloCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ASoloCharacter::OnRep_ReplicatedBasedMovement()
{
	//if (GetLocalRole() == ROLE_SimulatedProxy)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("OnRep_ReplicatedBasedMovement %lld"), UKismetSystemLibrary::GetFrameCount());
	//}
	Super::OnRep_ReplicatedBasedMovement();
}

FRotator ASoloCharacter::GetBaseAimRotation() const
{
	//DR: Default behavior uses camera rotation, which makes no sense for TopDownView.
	//I don't see why coupling this with the camera makes sense since the camera is already coupled with control rotation
	//this version works in both top-down and third person
	FRotator POVRot;
	if (Controller != nullptr && !InFreeCam())
	{
		return  Controller->GetControlRotation();
	}

	// If we have no controller, we simply use our rotation
	POVRot = GetActorRotation();

	// If our Pitch is 0, then use a replicated view pitch
	if (FMath::IsNearlyZero(POVRot.Pitch))
	{
		if (BlendedReplayViewPitch != 0.0f)
		{
			// If we are in a replay and have a blended value for playback, use that
			POVRot.Pitch = BlendedReplayViewPitch;
		}
		else
		{
			// Else use the RemoteViewPitch
			POVRot.Pitch = RemoteViewPitch;
			POVRot.Pitch = POVRot.Pitch * 360.0f / 255.0f;
		}
	}

	return POVRot;
}

bool ASoloCharacter::IsAccelerating() const
{
	if(GetLocalRole() == ENetRole::ROLE_SimulatedProxy)
	{
		return bReplicatedAccelerating;
	}
	float CurrentAccelerationSq = GetCharacterMovement()->GetCurrentAcceleration().SizeSquared2D();
	return CurrentAccelerationSq > 0;
}

void ASoloCharacter::SetAccelerating(bool bNewAccelerating)
{
	bReplicatedAccelerating = bNewAccelerating;
}

UAbilitySystemComponent* ASoloCharacter::GetAbilitySystemComponent() const
{
	return nullptr;
}


