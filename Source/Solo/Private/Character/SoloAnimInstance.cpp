// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SoloAnimInstance.h"

#include "DrawDebugHelpers.h"
#include "Character/SoloCharacter.h"
#include "Character/SoloCharacterMovementComponent.h"
#include "Util/SoloBlueprintFunctionLibrary.h"
#include "StriderMath.h"
#include "Animation/AnimNode_StateMachine.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Canvas.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetSystemLibrary.h"

FName DistanceCurveName = TEXT("DistanceCurve");
FName RotationCurveName = TEXT("RotationCurve");
FName DisableSpeedWarpingCurveName = TEXT("DisableSpeedWarping");
FName SMLocomotionName = TEXT("SM_Locomotion");
FName MoveStartStateName = TEXT("MoveStart");
FName MoveStopStateName = TEXT("MoveStop");
FName IdleStateName = TEXT("Idle");
FName TurnRecoveryStateName = TEXT("TurnRecovery");
FName TurnStateName = TEXT("TurnStart");
FName JumpStateName = TEXT("Jump");
FName RootBoneName = TEXT("root");

void USoloAnimBlueprintValidation::DoPostCompileValidation(FAnimBPCompileValidationParams& InParams) const
{
	UE_LOG(LogTemp, Log, TEXT("Test"));
}

USoloAnimInstance::USoloAnimInstance()
{
#if WITH_EDITORONLY_DATA
	PostCompileValidationClassName = USoloAnimBlueprintValidation::StaticClass();
#endif
	RunStartAnims = {nullptr, nullptr, nullptr, nullptr};
	WalkStartAnims = {nullptr, nullptr, nullptr, nullptr};
	MoveStopAnims = {nullptr, nullptr, nullptr, nullptr};
	MoveStartAnimOffsets = { 0.f, -90.f, -180.f, 90.f };
}

void USoloAnimInstance::DisplayDebugInstance(FDisplayDebugManager& DisplayDebugManager, float& Indent)
{
	Super::DisplayDebugInstance(DisplayDebugManager, Indent);
	FLinearColor TextYellow(0.86f, 0.69f, 0.f);
	FLinearColor TextWhite(0.9f, 0.9f, 0.9f);
	FLinearColor ActiveColor(0.1f, 0.6f, 0.1f);
	FLinearColor InactiveColor(0.2f, 0.2f, 0.2f);
	FLinearColor PoseSourceColor(0.5f, 0.25f, 0.5f);

	DisplayDebugManager.SetFont(GEngine->GetTinyFont());
	DisplayDebugManager.SetLinearDrawColor(TextWhite);

	for(TFieldIterator<FProperty> It(GetClass()); It; ++It)
	{
		const FString* CategoryPtr = It->FindMetaData(TEXT("Category"));
		FString Category = CategoryPtr ? *CategoryPtr : TEXT("");
		if(Category.Contains(TEXT("animcache")))
		{
			FString ValueStr;
			ValueStr += It->GetNameCPP() + ": ";
			It->ExportTextItem(ValueStr, It->ContainerPtrToValuePtr<void>(this), It->ContainerPtrToValuePtr<void>(this), nullptr, PPF_None);
			DisplayDebugManager.DrawString(ValueStr, Indent);
		}
	}
}

#if WITH_EDITOR
void USoloAnimInstance::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if(PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(USoloAnimInstance, JogBlend))
	{
		if(JogBlend)
		{
			MaxJogSpeed = JogBlend->GetBlendParameter(1).Max;
		}
		else
		{
			MaxJogSpeed = 0.f;
		}
	}
	if (PropertyChangedEvent.GetPropertyName() == GET_MEMBER_NAME_CHECKED(USoloAnimInstance, WalkBlend))
	{
		if (WalkBlend)
		{
			MaxWalkSpeed = WalkBlend->GetBlendParameter(1).Max;
		}
		else
		{
			MaxWalkSpeed = 0.f;
		}
	}
}
#endif

void USoloAnimInstance::NativeInitializeAnimation()
{
	SoloPawn = Cast<ASoloCharacter>(TryGetPawnOwner());
	if (!SoloPawn)
	{
		return;
	}
	SoloCharacterMovement = Cast<USoloCharacterMovementComponent>(SoloPawn->GetCharacterMovement());
	if(!SoloCharacterMovement)
	{
		return;
	}
	SoloPawn->CacheInitialMeshOffset(SoloPawn->GetMesh()->GetRelativeLocation(), SoloPawn->GetMesh()->GetRelativeRotation());
	CacheBasicInfo();
	if (auto* StateMachineDesc = GetStateMachineInstanceDesc(SMLocomotionName))
	{
		LocomotionStateMachine = GetStateMachineInstanceFromName(SMLocomotionName);
		MoveStartStateIndex = StateMachineDesc->FindStateIndex(MoveStartStateName);
		MoveStopStateIndex = StateMachineDesc->FindStateIndex(MoveStopStateName);
		IdleStateIndex = StateMachineDesc->FindStateIndex(IdleStateName);
		TurnRecoveryStateIndex = StateMachineDesc->FindStateIndex(TurnRecoveryStateName);
		TurnStateIndex = StateMachineDesc->FindStateIndex(TurnStateName);
		JumpStateIndex = StateMachineDesc->FindStateIndex(JumpStateName);
	}
	EnterIdle();
}

void USoloAnimInstance::CacheBasicInfo()
{
	PrevAcceleration = Acceleration;
	bPrevAccelerating = bAccelerating;
	PrevRotation = Rotation;
	PrevLocation = Location;
	PrevVelocity = Velocity;
	PrevSpeed = Speed;
	bPrevInAir = bInAir;

	bInAir = SoloCharacterMovement->MovementMode == EMovementMode::MOVE_Falling;
	bPrevWalking = bWalking;

	Location = SoloPawn->GetActorLocation();
	Rotation = SoloPawn->GetActorRotation();
	Velocity = SoloPawn->GetVelocity();
	Speed = Velocity.Size2D();
	Acceleration = SoloCharacterMovement->GetCurrentAcceleration();

	auto BaseMeshRotation = SoloPawn->GetBaseRotationOffset().Rotator();
	auto BaseMeshTranslation = SoloPawn->GetBaseTranslationOffset();
	auto CurrentMeshRotation = SoloPawn->GetMesh()->GetRelativeRotation();
	auto CurrentMeshTranslation = SoloPawn->GetMesh()->GetRelativeLocation();

	MeshRotationOffset = UKismetMathLibrary::NormalizedDeltaRotator(CurrentMeshRotation, BaseMeshRotation);
	MeshTranslationOffset = CurrentMeshTranslation - BaseMeshTranslation;
	Rotation = UKismetMathLibrary::ComposeRotators(Rotation, MeshRotationOffset);
	Location = Location + SoloPawn->GetMesh()->GetComponentTransform().TransformVectorNoScale(MeshTranslationOffset);
	
	if (!bInAir)
	{
		float AccVelDot = Acceleration.GetSafeNormal() | Velocity.GetSafeNormal();
		bAccelerating = !Acceleration.IsNearlyZero() && Speed > 0 && AccVelDot > 0;
		float MaxSpeedBasedOnAccel = FMath::Max(SoloCharacterMovement->GetMaxSpeed() * SoloCharacterMovement->GetAnalogInputModifier(), SoloCharacterMovement->GetMinAnalogSpeed());

		if(bAccelerating)
		{
			bWalking = MaxSpeedBasedOnAccel < MinJogSpeed;
		}
		else if(Speed == 0)
		{
			bWalking = false;
		}
	}
	else
	{
		bWalking = false;
		bAccelerating = false;
	}
	
	
	//if (SoloPawn->GetLocalRole() == ENetRole::ROLE_SimulatedProxy)
	//{
	//	DrawDebugSphere(GetWorld(), Location - MeshTranslationOffset, 10, 6, FColor::Green, false, 1.f);
	//	DrawDebugSphere(GetWorld(), Location, 20, 6, FColor::Blue, false, 1.f);

	//	DrawDebugLine(GetWorld(), Location, Location - MeshTranslationOffset, FColor::Emerald, false, 1.f);
	//}
	
	FRotator AimRotation = SoloPawn->GetBaseAimRotation();
	auto AimRotationLS = SoloPawn->ActorToWorld().InverseTransformVectorNoScale(AimRotation.Vector()).Rotation();
	PitchOffset = AimRotationLS.Pitch;
	YawOffset = AimRotationLS.Yaw;
}

void USoloAnimInstance::UpdateMoveDirection()
{
	RelativeDirection = USoloBlueprintFunctionLibrary::GetRotationRelativeToVelocityEx(Rotation, Velocity);
	RelativeDirection -= MoveRootOffset;
	RelativeDirection = FRotator::NormalizeAxis(RelativeDirection);

	PrevCardinalDirectionData = CardinalDirectionData;
	CardinalDirectionData = USoloBlueprintFunctionLibrary::GetNextCardinalDirectionEx(CardinalDirectionData.CardinalDirection, RelativeDirection, Orientation_StepDelta, Orientation_SkipDelta);
}

void USoloAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	if(!SoloPawn || !SoloCharacterMovement || !LocomotionStateMachine)
	{
		return;
	}
	CacheBasicInfo();

	if(SoloPawn->GetLocalRole() == ENetRole::ROLE_SimulatedProxy)
	{
		UE_LOG(LogTemp, VeryVerbose, TEXT("NativeUpdateAnimation %lld"), UKismetSystemLibrary::GetFrameCount());
	}

	if (bAccelerating)
	{
		const float JogSpeedScale = Speed / MaxJogSpeed;
		JogPlayRate = UStriderMath::CalculatePlayRate(JogSpeedScale, SpeedWarping_PlaybackWeight, SpeedWarping_MinPlayRate, SpeedWarping_MaxPlayRate);
		JogStrideScale = UStriderMath::CalculateStrideScale(JogSpeedScale, JogPlayRate);

		const float WalkSpeedScale = Speed / MaxWalkSpeed;
		WalkPlayRate = UStriderMath::CalculatePlayRate(WalkSpeedScale, SpeedWarping_PlaybackWeight, SpeedWarping_MinPlayRate, SpeedWarping_MaxPlayRate);
		WalkStrideScale = UStriderMath::CalculateStrideScale(WalkSpeedScale, WalkPlayRate);
	}
	
	UpdateMoveDirection();

	PrevJumpWeight = JumpWeight;
	JumpWeight = LocomotionStateMachine->GetStateWeight(JumpStateIndex);

	if(!bPrevInAir && bInAir)
	{
		EnterInAir();
		UpdateInAir(0.f);
	}
	else if(bInAir || PrevJumpWeight > 0 || bFinalizingJump)
	{
		UpdateInAir(DeltaSeconds);
	}

	if(!bInAir && (!bStartingMove && !bPrevAccelerating && bAccelerating) || bPendingStartingMove)
	{
		EnterStartingMovement();
		UpdateStartingMovement(0.f);
	}
	else if(bStartingMove || StartMoveAnim)
	{
		UpdateStartingMovement(DeltaSeconds);
	}

	auto PrevIdleWeight = IdleStateWeight;
	IdleStateWeight = LocomotionStateMachine->GetStateWeight(IdleStateIndex);

	if(!bMoved && Speed > 0)
	{
		bMoved = true;
	}

	if((!bInAir && Speed == 0 && PrevSpeed != 0) || bPrevInAir && !bInAir)
	{
		EnterIdle();
		UpdateIdle(0.f);
	}
	else if(!bInAir && Speed == 0 && !bTurn)
	{
		UpdateIdle(DeltaSeconds);
	}

	if (!bInAir && bTurn && !TurnAnim)
	{
		EnterTurn();
		UpdateTurn(0.f);
	}
	else if(TurnAnim)
	{
		UpdateTurn(DeltaSeconds);
	}
	else if (TurnRecoveryAnim)
	{
		UpdateTurnRecovery(DeltaSeconds);
	}

	if (!bInAir && !bStoppingMove && bPrevAccelerating && !bAccelerating)
	{
		EnterStoppingMovement();
		UpdateStoppingMovement(0.f);
	}
	else if (bStoppingMove || StopMoveAnim_A || StopMoveAnim_B)
	{
		UpdateStoppingMovement(DeltaSeconds);
	}


}

void USoloAnimInstance::EnterInAir()
{
	//refactor this
	EnterStoppingMovement();
	EndTurn();
	TurnAnim = nullptr;
	TurnRecoveryAnim = nullptr;
	
	IdleRootOffset = 0.f;
	IdleYaw = 0.f;
	MoveRootOffset = 0.f;
	
	StartApexAnimAt = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(JumpApex, DistanceCurveName, 0.f);
	StartPreLandAt = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(JumpPreLand, DistanceCurveName, 0.f);
	JumpPreLandRootOffset = 0.f;
	JumpApexTime = 0.f;
	JumpAnimTime = 0.f;
	JumpPreLandTime = 0.f;
	bFinalizingJump = false;
	JumpPelvisTransform = FTransform::Identity;
	StartedWarpingJumpAt = 0.f;
	if(Velocity.Z > PrevVelocity.Z)
	{
		bJump = true;
		bFalling = false;
		bJumpApex = false;
		bJumpPreLand = false;
	}
	else
	{
		bFalling = true;
		bJump = false;
		bJumpApex = false;
		bJumpPreLand = false;
	}
	InAirStartLocation = PrevLocation;
}

void USoloAnimInstance::UpdateInAir(float DeltaTime)
{
	if (bJump)
	{
		float JumpHeight = Location.Z - InAirStartLocation.Z;
		float JumpAnimTimeTemp = USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(JumpAnim, DistanceCurveName, JumpHeight);
		int32 PelvisIndex = GetOwningComponent()->GetBoneIndex(TEXT("pelvis"));
		FTransform PrevJumpPelvisTransform = JumpPelvisTransform;
		JumpAnim->GetBoneTransform(JumpPelvisTransform, PelvisIndex, JumpAnimTime, false);
		float JumpPlayRate = 1.f;
		if (DeltaTime > 0)
		{
			float PelvisChange = FMath::Abs(PrevJumpPelvisTransform.GetLocation().Z - JumpPelvisTransform.GetLocation().Z);
			//UE_LOG(LogTemp, Log, TEXT("%f %f"), PelvisChange, JumpAnimTime);
			if(PelvisChange < 1.f && JumpAnimTime > JumpAnim->SequenceLength * 0.5f)
			{
				if (StartedWarpingJumpAt == 0.f)
				{
					StartedWarpingJumpAt = JumpAnimTime;
				}
				JumpPlayRate = (JumpAnim->SequenceLength - JumpAnimTime) / (JumpAnim->SequenceLength - StartedWarpingJumpAt);
				JumpAnimTime = FMath::Clamp(JumpAnimTime + DeltaTime * JumpPlayRate, 0.f, JumpAnim->SequenceLength);
			}
			else
			{
				JumpAnimTime = JumpAnimTimeTemp;
			}
		}
		else
		{
			JumpAnimTime = JumpAnimTimeTemp;
		}
		//UE_LOG(LogTemp, Log, TEXT("playrate %f"), JumpPlayRate);
	}
	if(bJump && !bJumpApex && !bJumpPreLand && !bFalling)
	{
		float GravityZ = SoloCharacterMovement->GetGravityZ();
		float TimeToApex = FMath::Abs(Velocity.Z / GravityZ);
		ApexLocation = Location + Velocity * TimeToApex + FVector{ 0, 0, GravityZ } *TimeToApex * TimeToApex / 2;
		//DrawDebugSphere(GetWorld(), ApexLocation, 20, 20, FColor::Blue, false, 3);
		float DistanceToApex = Location.Z - ApexLocation.Z;
		bool bMovingToApex = Velocity.Z > 0;
		if(!bMovingToApex || (bMovingToApex && DistanceToApex >= StartApexAnimAt))
		{
			bJumpApex = true;
			bJump = false;
		}
	}
	if(bJumpApex)
	{
		float DistanceToApex = FMath::Min(Location.Z - ApexLocation.Z, 0.f);
		bool bMovingToApex = Velocity.Z > 0;
		float ApexAnimVal = DistanceToApex * (bMovingToApex ? 1 : -1);
		JumpApexTime = USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(JumpApex, DistanceCurveName, ApexAnimVal);
	}
	if(Velocity.Z <= 0.f && !bFalling && !bJumpPreLand)
	{
		bFalling = true;
		bJump = false;
	}
	if(bFalling)
	{
		FPredictProjectilePathParams Params;
		Params.ActorsToIgnore = { SoloPawn };
		//Params.DrawDebugTime = 3.f;
		//Params.DrawDebugType = EDrawDebugTrace::ForDuration;
		Params.LaunchVelocity = Velocity;
		Params.MaxSimTime = 3.f;
		Params.OverrideGravityZ = SoloCharacterMovement->GetGravityZ();
		Params.ProjectileRadius = SoloPawn->GetCapsuleComponent()->GetScaledCapsuleRadius() * 0.9;
		Params.SimFrequency = 10;
		Params.StartLocation = Location - FVector{ 0, 0, SoloPawn->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() / 2.f };
		//Params.TraceChannel = ECollisionChannel::ECC_WorldStatic;
		Params.ObjectTypes = { EObjectTypeQuery::ObjectTypeQuery1, EObjectTypeQuery::ObjectTypeQuery2, EObjectTypeQuery::ObjectTypeQuery5, EObjectTypeQuery::ObjectTypeQuery6 };
		Params.bTraceWithChannel = false;
		Params.bTraceWithCollision = true;
		FPredictProjectilePathResult Result;
		bool FoundFloor = UGameplayStatics::PredictProjectilePath(this, Params, Result);
		float HitDir = (Result.HitResult.Normal | FVector::UpVector);
		
		
		FoundFloor = FoundFloor && HitDir >= FMath::Cos(SoloCharacterMovement->GetWalkableFloorAngle());
		if(FoundFloor)
		{
			InAirLandLocation = Result.HitResult.Location;
			DrawDebugSphere(GetWorld(), InAirLandLocation, 20, 20, FColor::Red, false, 3);
			float JumpHeight = InAirLandLocation.Z - Location.Z;
			if (JumpHeight >= StartPreLandAt)
			{
				bJumpPreLand = true;
			}
		}
		
	}
	if(bJumpPreLand)
	{
		float JumpHeight = InAirLandLocation.Z - Location.Z;
		JumpPreLandTime = USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(JumpPreLand, DistanceCurveName, JumpHeight);
		float MinSupportedOffset = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(JumpPreLand, DistanceCurveName, JumpPreLand->SequenceLength);
		if(JumpHeight > MinSupportedOffset)
		{
			JumpPreLandRootOffset = -MinSupportedOffset + JumpHeight;
		}
	}
	if(!bInAir && JumpWeight == 0)
	{
		EndInAir();
	}
	
}
void USoloAnimInstance::EndInAir()
{
	bFalling = false;
	bJump = false;
	bJumpApex = false;
	bJumpPreLand = false;
	JumpPreLandRootOffset = 0.f;
	JumpApexTime = 0.f;
	JumpAnimTime = 0.f;
	JumpPreLandTime = 0.f;
	bFinalizingJump = false;
}

void USoloAnimInstance::EnterTurn()
{
	if (TurnAnim == nullptr)
	{
		if(FMath::Abs(IdleRootOffset) > 180 - Turn90At)
		{
			TurnAnim = TurnAnims[FMath::Sign(IdleRootOffset) * -180];
			TurnPlayRate = FMath::Abs(IdleRootOffset);
			//TurnPlayRate = UKismetMathLibrary::MapRangeClamped(TurnPlayRate, 180 - Turn90At, 180, 1.f, MaxTurnAnimPlayRate);
			TurnPlayRate = UKismetMathLibrary::MapRangeClamped(TurnPlayRate, 180 - Turn90At, 180, 0.f, 1.f);
			TurnPlayRate = UKismetMathLibrary::Ease(1.f, FMath::Max(1.f, MaxTurnAnimPlayRate / TurnAnim->RateScale), TurnPlayRate, EEasingFunc::EaseIn);
			TurnPlayRate = TurnPlayRate * TurnAnim->RateScale;
			TurnAnimExpectedTurn = 180;
		}
		else
		{
			TurnAnim = TurnAnims[FMath::Sign(IdleRootOffset) * -90];
			TurnPlayRate = FMath::Abs(IdleRootOffset);
			//TurnPlayRate = UKismetMathLibrary::MapRangeClamped(TurnPlayRate, Turn90At, 180 - Turn90At, 1.f, MaxTurnAnimPlayRate);
			TurnPlayRate = UKismetMathLibrary::MapRangeClamped(TurnPlayRate, Turn90At, 180 - Turn90At, 0.f, 1.f);
			TurnPlayRate = UKismetMathLibrary::Ease(1.f, FMath::Max(1.f, MaxTurnAnimPlayRate / TurnAnim->RateScale), TurnPlayRate, EEasingFunc::EaseIn);
			TurnPlayRate = TurnPlayRate * TurnAnim->RateScale;
			TurnAnimExpectedTurn = 90;
		}
		TurnAnimPosition = 0.f;
		//if (TurnPlayRate > (MaxTurnAnimPlayRate + 1.f) / 2.f) //if we need a fast turn:
		//{
		//	TurnAnimPosition = USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(TurnAnim, DistanceCurveName, -TurnAnimExpectedTurn + KINDA_SMALL_NUMBER);
		//}
		TurnDir = FMath::Sign(IdleRootOffset) * -1;
		bTurningForward = true;
	}
}
void USoloAnimInstance::UpdateTurn(float DeltaTime)
{
	if (Speed > 0)
	{
		EndTurn();
		float TurnWeight = LocomotionStateMachine->GetStateWeight(TurnStateIndex);
		if (TurnWeight == 0)
		{
			TurnAnim = nullptr;
		}
	}
	else
	{
		float DesiredTurnDir = FMath::Sign(IdleRootOffset) * -1;
		float PrevEvaluatedValue = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(TurnAnim, RotationCurveName, TurnAnimPosition);
		if (bTurningForward && DesiredTurnDir != TurnDir && (TurnAnimExpectedTurn - FMath::Abs(PrevEvaluatedValue)) < TurnAnimExpectedTurn / 2.f)
		{
			TurnDir = DesiredTurnDir;
			bTurningForward = false;
		}
		if (bTurningForward)
		{
			float PrevTurnAnimPosition = TurnAnimPosition;
			TurnAnimPosition += DeltaTime * TurnPlayRate;
			TurnAnimPosition = FMath::Clamp(TurnAnimPosition, 0.f, TurnAnim->SequenceLength);
			float PrevDistanceCurveValue = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(TurnAnim, RotationCurveName, PrevTurnAnimPosition);
			float DistanceCurveValue = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(TurnAnim, RotationCurveName, TurnAnimPosition);
			float DistanceCurveDiff = DistanceCurveValue - PrevDistanceCurveValue;
			IdleYaw = IdleYaw + DistanceCurveDiff * TurnDir;
			IdleYaw = FRotator::NormalizeAxis(IdleYaw);
			CalcRootOffset();
			if (DistanceCurveValue > -TurnAnimExpectedTurn * 0.05)
			{
				EnterTurnRecovery();
			}
		}
		else
		{
			float PrevTurnAnimPosition = TurnAnimPosition;
			TurnAnimPosition -= DeltaTime * TurnPlayRate;
			TurnAnimPosition = FMath::Clamp(TurnAnimPosition, 0.f, TurnAnim->SequenceLength);
			float PrevDistanceCurveValue = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(TurnAnim, RotationCurveName, PrevTurnAnimPosition);
			float DistanceCurveValue = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(TurnAnim, RotationCurveName, TurnAnimPosition);
			float DistanceCurveDiff = DistanceCurveValue - PrevDistanceCurveValue;
			IdleYaw = IdleYaw + DistanceCurveDiff * TurnDir * -1;
			IdleYaw = FRotator::NormalizeAxis(IdleYaw);
			CalcRootOffset();
			if (DistanceCurveValue <= -TurnAnimExpectedTurn * 0.95)
			{
				EndTurn();
				float TurnWeight = LocomotionStateMachine->GetStateWeight(TurnStateIndex);
				if (TurnWeight == 0)
				{
					TurnAnim = nullptr;
				}
			}
		}
	}
}

void USoloAnimInstance::EndTurn()
{
	bTurn = false;
}


void USoloAnimInstance::EnterIdle()
{
	if (bMoved)
	{
		IdleRootOffset = 0;
	}
	if (IdleRootOffset == 0)
	{
		IdleYaw = Rotation.Yaw;
	}
}

void USoloAnimInstance::CalcRootOffset()
{
	float PrevIdleRootOffset = IdleRootOffset;
	IdleRootOffset = FRotator::NormalizeAxis(IdleYaw - Rotation.Yaw);
	float DeltaAngle = FMath::FindDeltaAngleDegrees(PrevIdleRootOffset, IdleRootOffset);
	float ABSDeltaAngle = FMath::Abs(FMath::FindDeltaAngleDegrees(PrevIdleRootOffset, IdleRootOffset));
	float ABSAngle = FMath::Abs(PrevIdleRootOffset - IdleRootOffset);
	if (ABSDeltaAngle < 180.f && ABSAngle > 180.f)
	{
		IdleRootOffset = PrevIdleRootOffset + DeltaAngle;
	}
}

void USoloAnimInstance::EnterTurnRecovery()
{
	bTurn = false;
	TurnRecoveryAnim = TurnAnim;
	TurnRecoveryAnimPosition = TurnAnimPosition;
	TurnAnim = nullptr;
}
void USoloAnimInstance::UpdateTurnRecovery(float DeltaTime)
{
	TurnRecoveryAnimPosition += DeltaTime;
	TurnRecoveryAnimPosition = FMath::Clamp(TurnRecoveryAnimPosition, 0.f, TurnRecoveryAnim->SequenceLength);
	float TurnRecoveryWeight = LocomotionStateMachine->GetStateWeight(TurnRecoveryStateIndex);
	if(TurnRecoveryWeight == 0)
	{
		TurnRecoveryAnim = nullptr;
	}
}


void USoloAnimInstance::UpdateIdle(float DeltaTime)
{
	CalcRootOffset();
	if (FMath::Abs(IdleRootOffset) > Turn90At) //turn right, turn left
	{
		bTurn = true;
	}
	if(Speed == 0)
	{
		bMoved = false;
	}
}

void USoloAnimInstance::EnterStartingMovement()
{
	StartMoveIdleRootOffset = IdleRootOffset;
	//StartMoveIdleRootOffset = FRotator::NormalizeAxis(StartMoveIdleRootOffset);
	MoveRootOffset = StartMoveIdleRootOffset;
	UpdateMoveDirection(); //recalculate this here after we set the initial move root offset
	
	if(StartMoveAnim)
	{
		bPendingStartingMove = true;
		StartMovingLocation = PrevLocation;
		StartMovementRelativeDirection = RelativeDirection;
	}
	else
	{
		bStartingMove = true;
		if (!bPendingStartingMove)
		{
			StartMovingLocation = PrevLocation;
			StartMovementRelativeDirection = RelativeDirection;
		}
		bPendingStartingMove = false;

		if(!bWalking)
		{
			StartMoveAnim_A = RunStartAnims[(int32)CardinalDirectionData.CardinalDirection];
			StartMoveAnim_B = RunStartAnims[(int32)CardinalDirectionData.SecondaryCardinalDirection];
			StartMoveAnim = RunStartAnims[(int32)CardinalDirectionData.CardinalDirection];
			StartMoveStrideScale = JogStrideScale;
		}
		else
		{
			StartMoveAnim_A = WalkStartAnims[(int32)CardinalDirectionData.CardinalDirection];
			StartMoveAnim_B = WalkStartAnims[(int32)CardinalDirectionData.SecondaryCardinalDirection];
			StartMoveAnim = WalkStartAnims[(int32)CardinalDirectionData.CardinalDirection];
			StartMoveStrideScale = WalkStrideScale;
		}
		
		StartMovementAnimOffset = MoveStartAnimOffsets[(int32)CardinalDirectionData.CardinalDirection];
		StartMovementAnimAlpha = CardinalDirectionData.Alpha;
		DistanceMovedFromStart = 0.f;
		VisualDistanceMovedFromStart = 0.f;
		StartMoveSpeedWarpingAlpha = 0.f;
		StartMoveFirstStepAlpha = 0.f;
		StartTimeA = StartTimeB = StartTime = 0.f;
	}
}


void USoloAnimInstance::UpdateStartingMovement(float DeltaTime)
{
	bool bOrientationChanged = StartMoveFirstStepAlpha > 0.f && PrevCardinalDirectionData.CardinalDirection != ECardinalDirection::Invalid &&
		(PrevCardinalDirectionData.CardinalDirection != CardinalDirectionData.CardinalDirection ||
			(PrevCardinalDirectionData.SecondaryCardinalDirection != CardinalDirectionData.SecondaryCardinalDirection && (PrevCardinalDirectionData.Alpha > 0.05 || CardinalDirectionData.Alpha > 0.05)));

	bool bChangedBetweenRunningWalking = DeltaTime > 0? bWalking != bPrevWalking : false; //no change on first update
	//bool bTooSlow = StartMoveFirstStepAlpha > 0.f && bWalking && PrevSpeed >= Speed;
	if (!bStartingMove || bOrientationChanged /*|| bTooSlow*/ || bChangedBetweenRunningWalking || !bAccelerating)
	{
		bStartingMove = false;
		PrevMoveStartStateWeight = MoveStartStateWeight;
		MoveStartStateWeight = LocomotionStateMachine->GetStateWeight(MoveStartStateIndex);
		if (MoveStartStateWeight == 0.f)
		{
			EndStartingMovement();
			return;
		}
	}

	if(bPendingStartingMove)
	{
		return;
	}
	
	if(bStartingMove)
	{
		StartMoveStrideScale = bWalking? WalkStrideScale : JogStrideScale;
		StartMovementAnimAlpha = CardinalDirectionData.Alpha;
		StartMovementRelativeDirection = RelativeDirection;
	}

	if(StartMoveFirstStepAlpha == 0.f) //allow changing start anim until we start blending in speed warped anim
	{
		DistanceMovedFromStart = (Location - StartMovingLocation).Size2D();
		VisualDistanceMovedFromStart = DistanceMovedFromStart;
		if(!bWalking)
		{
			StartMoveAnim_A = RunStartAnims[(int32)CardinalDirectionData.CardinalDirection];
			StartMoveAnim_B = RunStartAnims[(int32)CardinalDirectionData.SecondaryCardinalDirection];
			StartMoveAnim = RunStartAnims[(int32)CardinalDirectionData.CardinalDirection];
		}
		else
		{
			StartMoveAnim_A = WalkStartAnims[(int32)CardinalDirectionData.CardinalDirection];
			StartMoveAnim_B = WalkStartAnims[(int32)CardinalDirectionData.SecondaryCardinalDirection];
			StartMoveAnim = WalkStartAnims[(int32)CardinalDirectionData.CardinalDirection];
		}
		StartMovementAnimOffset = MoveStartAnimOffsets[(int32)CardinalDirectionData.CardinalDirection];
	}
	else
	{
		float DiffDistance = (Location - PrevLocation).Size2D();
		DistanceMovedFromStart += DiffDistance;
		VisualDistanceMovedFromStart += DiffDistance / FMath::Lerp(1.f, StartMoveStrideScale, StartMoveSpeedWarpingAlpha);
	}
	
	StartTimeA = USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(StartMoveAnim_A, DistanceCurveName, DistanceMovedFromStart);
	StartTimeB = USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(StartMoveAnim_B, DistanceCurveName, DistanceMovedFromStart);
	float FirstStepA = FMath::Max(1 - USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(StartMoveAnim_A, DisableSpeedWarpingCurveName, StartTimeA), 0.f);
	float FirstStepB = FMath::Max(1 - USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(StartMoveAnim_B, DisableSpeedWarpingCurveName, StartTimeB), 0.f);
	StartMoveFirstStepAlpha = FMath::Sqrt(FirstStepA * FirstStepB);
	StartTime = USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(StartMoveAnim, DistanceCurveName, VisualDistanceMovedFromStart);
	StartMoveSpeedWarpingAlpha = FMath::Max(1 - USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(StartMoveAnim, DisableSpeedWarpingCurveName, StartTime), 0.f);
	MoveRootOffset = StartMoveIdleRootOffset * (1 - StartMoveSpeedWarpingAlpha);
}

void USoloAnimInstance::EndStartingMovement()
{
	bStartingMove = false;
	MoveStartStateWeight = 0.f;
	PrevMoveStartStateWeight = 0.f;
	StartMoveAnim = nullptr;
}

void USoloAnimInstance::EnterStoppingMovement()
{
	bStoppingMove = true;
	StopMoveLocation = SoloCharacterMovement->PredictStopLocation();
	FVector LocationToStopDir = (StopMoveLocation - Location).GetSafeNormal();
	float DirDot = LocationToStopDir | Velocity.GetSafeNormal();
	if (DirDot <= 0)
	{
		StopMoveLocation = Location;
	}
	//if (SoloPawn->GetLocalRole() == ENetRole::ROLE_SimulatedProxy)
	//{
	//	DrawDebugSphere(GetWorld(), StopMoveLocation, 20, 20, FColor::Red, false, 3);
	//}
	StopMoveAnim_A = MoveStopAnims[(int32)CardinalDirectionData.CardinalDirection];
	StopMoveAnim_B = MoveStopAnims[(int32)CardinalDirectionData.SecondaryCardinalDirection];
	StopMoveAlpha = CardinalDirectionData.Alpha;

	float DistanceToStop = (StopMoveLocation - Location).Size2D() * -1;
	StopTimeA = USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(StopMoveAnim_A, DistanceCurveName, DistanceToStop);
	StopTimeB = USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(StopMoveAnim_B, DistanceCurveName, DistanceToStop);
	bStoppingUsingDistance = true;
}


void USoloAnimInstance::UpdateStoppingMovement(float DeltaTime)
{
	if (bStartingMove || bTurn || bAccelerating || !bStoppingMove)
	{
		bStoppingMove = false;
		PrevMoveStopStateWeight = MoveStopStateWeight;
		MoveStopStateWeight = LocomotionStateMachine->GetStateWeight(MoveStopStateIndex);
		if (MoveStopStateWeight == 0 && PrevMoveStopStateWeight > 0)
		{
			EndStoppingMovement();
			return;
		}
	}

	
	if (bStoppingMove)
	{
		float DistanceToStop = (StopMoveLocation - Location).Size2D() * -1;
		FVector LocationToStopDir = (StopMoveLocation - Location).GetSafeNormal();
		float DirDot = LocationToStopDir | Velocity.GetSafeNormal();
		if (DirDot <= 0 && DistanceToStop != 0)
		{
			DistanceToStop = 0;
			StopMoveLocation = Location;
		}
		float AnimCurveValueA = USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(StopMoveAnim_A, DistanceCurveName, DistanceToStop);
		float AnimCurveValueB = USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(StopMoveAnim_A, DistanceCurveName, DistanceToStop);
		if(DeltaTime > 0)
		{
			float DistanceStopPlayRate = (AnimCurveValueA - StopTimeA) / DeltaTime;
			if(bStoppingUsingDistance && (DistanceStopPlayRate < SpeedWarping_MinPlayRate || DistanceStopPlayRate > SpeedWarping_MaxPlayRate))
			{
				bStoppingUsingDistance = false;
			}
			if(bStoppingUsingDistance)
			{
				StopTimeA = AnimCurveValueA;
				StopTimeB = AnimCurveValueB;
			}
			else
			{
				StopTimeA += DeltaTime;
				StopTimeB += DeltaTime;
				StopTimeA = FMath::Clamp(StopTimeA, 0.f, StopMoveAnim_A->SequenceLength);
				StopTimeB = FMath::Clamp(StopTimeB, 0.f, StopMoveAnim_B->SequenceLength);
			}
		}
		else
		{
			StopTimeA = AnimCurveValueA;
			StopTimeB = AnimCurveValueB;
		}

	}
	else
	{
		StopTimeA += DeltaTime;
		StopTimeB += DeltaTime;
		StopTimeA = FMath::Clamp(StopTimeA, 0.f, StopMoveAnim_A->SequenceLength);
		StopTimeB = FMath::Clamp(StopTimeB, 0.f, StopMoveAnim_B->SequenceLength);
	}
}

void USoloAnimInstance::EndStoppingMovement()
{
	bStoppingMove = false;
	MoveStopStateWeight = 0.f;
	PrevMoveStopStateWeight = 0.f;
	bStoppingMove = false;
	StopMoveAnim_A = nullptr;
	StopMoveAnim_B = nullptr;
}

