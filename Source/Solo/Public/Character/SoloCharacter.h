// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "Ability/SoloAbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "SoloCharacter.generated.h"

class USoloGameplayAbility;

UCLASS()
class SOLO_API ASoloCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASoloCharacter(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(BlueprintReadWrite, Category = "Debug")
	FVector2D FakeMoveInput;
	FVector2D MoveInput;
	FVector2D LookInput;
	FVector2D LookRateInput;
	FVector2D CursorOffset;

	void MoveRight(float Val);
	void MoveForward(float Val);
	void LookUp(float Val);
	void LookUpRate(float Val);
	void Turn(float Val);
	void TurnRate(float Val);
	
	virtual void BeginPlay() override;

	UPROPERTY(Replicated, ReplicatedUsing="OnRep_ReplicatedAccelerating")
	bool bReplicatedAccelerating = false;
	UPROPERTY(Transient)
	USoloAbilitySystemComponent* AbilitySystemComponent = nullptr;
	void InitAbilitySystemComponent();
	UFUNCTION()
	void OnRep_ReplicatedAccelerating();
	void BindASCInput();
	bool bASCInputBound = false;

	UFUNCTION(BlueprintImplementableEvent)
	void UpdateCrosshair(const FVector& Vector);

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "SoloCharacter|Abilities")
	TArray<TSubclassOf<USoloGameplayAbility>> StartupAbilities;
public:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	void PostProcessInput(const float DeltaTime, const bool bGamePaused);
	
	virtual void OnRep_Controller() override;
	virtual void OnRep_PlayerState() override;
	virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DebugDisplay, float& YL, float& YPos) override;
	virtual void GetActorEyesViewPoint(FVector& Location, FRotator& Rotation) const override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void OnRep_ReplicatedBasedMovement() override;
	virtual FRotator GetBaseAimRotation() const override;
	bool IsAccelerating() const;
	void SetAccelerating(bool bNewAccelerating);
	
	//BEGIN IAbilitySystemInterface
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;
	//END IAbilitySystemInterface
};
