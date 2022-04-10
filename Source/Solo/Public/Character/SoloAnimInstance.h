// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"


#include "AnimBlueprintPostCompileValidation.h"
#include "SoloCharacter.h"
#include "SoloCharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Util/SoloBlueprintFunctionLibrary.h"

#include "SoloAnimInstance.generated.h"

UCLASS()
class SOLO_API USoloAnimBlueprintValidation : public UAnimBlueprintPostCompileValidation
{
	GENERATED_BODY()
public:
	virtual void DoPostCompileValidation(FAnimBPCompileValidationParams& InParams) const override;

};


UCLASS()
class SOLO_API USoloAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	USoloAnimInstance();
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void DisplayDebugInstance(FDisplayDebugManager& DisplayDebugManager, float& Indent) override;
private:
	void UpdateMoveDirection();
	
	void EnterIdle();
	void UpdateIdle(float DeltaTime);

	void CalcRootOffset();
	
	void EnterTurn();
	void UpdateTurn(float DeltaTime);
	void EndTurn();
	
	void EnterTurnRecovery();
	void UpdateTurnRecovery(float DeltaTime);
	
	void EnterStartingMovement();
	void UpdateStartingMovement(float DeltaTime);
	void EndStartingMovement();
	
	void EnterStoppingMovement();
	void UpdateStoppingMovement(float DeltaTime);
	void EndStoppingMovement();

	void EnterInAir();
	void UpdateInAir(float DeltaTime);
	void EndInAir();

	void CacheBasicInfo();
public:

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	UBlendSpace* JogBlend = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float MinJogSpeed = 200.f;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float MaxJogSpeed = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	UBlendSpace* WalkBlend = nullptr;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float MaxWalkSpeed = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (EditFixedSize))
	TArray<UAnimSequence*> RunStartAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (EditFixedSize))
	TArray<UAnimSequence*> WalkStartAnims;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config", meta = (EditFixedSize))
	TArray<UAnimSequence*> MoveStopAnims;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Config")
	TArray<float> MoveStartAnimOffsets;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float SpeedWarping_PlaybackWeight = 0.5f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float SpeedWarping_MinPlayRate = 0.8f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float SpeedWarping_MaxPlayRate = 1.2f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float Orientation_StepDelta = 60.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float Orientation_SkipDelta = 135.f;



	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	FRotator MeshRotationOffset = FRotator::ZeroRotator;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	FVector MeshTranslationOffset = FVector::ZeroVector;

	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	FVector PrevLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	FVector Location = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	FVector PrevVelocity = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	FVector Velocity = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	float PrevSpeed = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	float Speed = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	FVector PrevAcceleration = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	FVector Acceleration = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	bool bPrevAccelerating = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	bool bAccelerating = false;
	UPROPERTY(BlueprintReadOnly, Category = "Cache")
	ASoloCharacter* SoloPawn = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "Cache")
	USoloCharacterMovementComponent* SoloCharacterMovement = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	FRotator PrevRotation;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	FRotator Rotation;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	float PitchOffset = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	float YawOffset = 0.f;
	
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	FCardinalDirectionData CardinalDirectionData;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Prev")
	FCardinalDirectionData PrevCardinalDirectionData;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	float JogStrideScale = 0;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	float JogPlayRate = 0;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	float WalkStrideScale = 0;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	float WalkPlayRate = 0;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	float RelativeDirection = 0;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache")
	bool bWalking = false;
	bool bPrevWalking = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	FVector	StartMovingLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float DistanceMovedFromStart = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float VisualDistanceMovedFromStart = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StartMoveSpeedWarpingAlpha = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StartMovementRelativeDirection = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	UAnimSequence* StartMoveAnim_A = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	UAnimSequence* StartMoveAnim_B = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StartMovementAnimAlpha = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	UAnimSequence* StartMoveAnim = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StartMovementAnimOffset = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StartTimeA = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StartTimeB = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StartTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StartMoveFirstStepAlpha = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StartMoveStrideScale = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StartMoveIdleRootOffset = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float MoveRootOffset = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMove")
	bool bStartingMove = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMove")
	bool bPendingStartingMove = false;
	
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StoptMovement")
	bool bStoppingMove = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StoptMovement")
	FVector StopMoveLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StoptMovement")
	UAnimSequence* StopMoveAnim_A = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StoptMovement")
	UAnimSequence* StopMoveAnim_B = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StoptMovement")
	float StopMoveAlpha = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StopTimeA = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	float StopTimeB = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|StartMovement")
	bool bStoppingUsingDistance = true;
	
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	bool bMoved = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	float IdleYaw = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	float IdleRootOffset = 0.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float Turn90At = 70.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	float MaxTurnAnimPlayRate = 2.f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	TMap<int32, UAnimSequence*> TurnAnims;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	UAnimSequence* TurnAnim = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	UAnimSequence* TurnRecoveryAnim = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	float TurnPlayRate = 1.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	float TurnDir = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	float TurnAnimPosition = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	float TurnRecoveryAnimPosition = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	bool bTurn = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	bool bTurningForward = true;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Idle")
	float TurnAnimExpectedTurn = 0.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	UAnimSequence* JumpAnim = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	UAnimSequence* JumpApex = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Config")
	UAnimSequence* JumpPreLand = nullptr;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	bool bPrevInAir = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	bool bInAir = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	bool bJump = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	float JumpAnimTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	bool bJumpApex = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	float JumpApexTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	bool bJumpPreLand = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	float JumpPreLandTime = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	float JumpPreLandRootOffset = 0.f;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	bool bFalling = false;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	FVector InAirStartLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	FVector InAirApexLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadOnly, Category = "AnimCache|Jump")
	FVector InAirLandLocation = FVector::ZeroVector;
	UPROPERTY(BlueprintReadWrite, Category = "AnimCache|Jump")
	bool bFinalizingJump = false;
	float StartApexAnimAt;
	float StartPreLandAt;
	float PrevDistanceFromApex = 0.f;
	FVector ApexLocation;
	FTransform JumpPelvisTransform;
	float StartedWarpingJumpAt;
private:
	int32 MoveStartStateIndex = INDEX_NONE;
	int32 MoveStopStateIndex = INDEX_NONE;
	int32 IdleStateIndex = INDEX_NONE;
	int32 TurnRecoveryStateIndex = INDEX_NONE;
	int32 TurnStateIndex = INDEX_NONE;
	int32 JumpStateIndex = INDEX_NONE;

	float MoveStartStateWeight = 0.f;
	float PrevMoveStartStateWeight = 0.f;

	float JumpWeight = 0.f;
	float PrevJumpWeight = 0.f;
	
	float MoveStopStateWeight = 0.f;
	float PrevMoveStopStateWeight = 0.f;
	float IdleStateWeight = 0.f;

public:




	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
