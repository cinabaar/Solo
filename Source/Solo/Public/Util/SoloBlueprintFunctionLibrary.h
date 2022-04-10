// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#if WITH_EDITOR
#include "IDetailCustomization.h"
#endif
#include "SoloBlueprintFunctionLibrary.generated.h"

class UAimOffsetBlendSpace;

UENUM(BlueprintType)
enum class ECardinalDirection : uint8
{
	North = 0,
	East = 1,
	South = 2,
	West = 3,
	MAX = 4 UMETA(Hidden),
	Invalid = 255 UMETA(DisplayName = "Invalid"),
};

FString EnumToString(const ECardinalDirection Value);

UCLASS(BlueprintType)
class SOLO_API USoloAnimSetDefinition : public UDataAsset
{
	GENERATED_BODY()
public:
	USoloAnimSetDefinition();

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimSequence* Idle = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAimOffsetBlendSpace* AimOffset = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Walk_{Dir}"))
	TMap<ECardinalDirection, UAnimSequence*> WalkAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Run_{Dir}"))
	TMap<ECardinalDirection, UAnimSequence*> RunAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Sprint_{Dir}"))
	TMap<ECardinalDirection, UAnimSequence*> SprintAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Walk_{Dir}_Start"))
	TMap<ECardinalDirection, UAnimSequence*> WalkStartAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Run_{Dir}_Start"))
	TMap<ECardinalDirection, UAnimSequence*> RunStartAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Sprint_{Dir}_Start"))
	TMap<ECardinalDirection, UAnimSequence*> SprintStartAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Walk_{Dir}_Stop"))
	TMap<ECardinalDirection, UAnimSequence*> WalkStopAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Run_{Dir}_Stop"))
	TMap<ECardinalDirection, UAnimSequence*> RunStopAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Sprint_{Dir}_Stop"))
	TMap<ECardinalDirection, UAnimSequence*> SprintStopAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Jump_Start"))
	UAnimSequence* JumpStart = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Jump_Apex"))
	UAnimSequence* JumpApex = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Jump_Loop"))
	UAnimSequence* JumpLoop = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Jump_Land"))
	UAnimSequence* JumpLand = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize, NameFormat = "Jump_Recovery_Add"))
	UAnimSequence* JumpRecoveryAdd = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize))
	TMap<int32, UAnimSequence*> TurnAnims;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

#if WITH_EDITOR

class FAnimSetCustomization: public IDetailCustomization
{
public:
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	static TSharedRef< IDetailCustomization > MakeInstance();
};
#endif

USTRUCT(BlueprintType)
struct FCardinalDirectionData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECardinalDirection CardinalDirection = ECardinalDirection::Invalid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ECardinalDirection SecondaryCardinalDirection = ECardinalDirection::Invalid;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Alpha = 0;
};

USTRUCT(BlueprintType)
struct FAnimStateMachineHandle
{
	GENERATED_BODY()
public:
	const struct FBakedAnimationStateMachine* StateMachineDesc = nullptr;
	const struct FAnimNode_StateMachine* StateMachine = nullptr;
};

USTRUCT(BlueprintType)
struct FAnimStateMachineStateHandle
{
	GENERATED_BODY()
public:
	const struct FAnimNode_StateMachine* StateMachine = nullptr;
	int32 StateIndex = INDEX_NONE;
};
/**
 * 
 */
UCLASS()
class SOLO_API USoloBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** Given an input Cardinal Direction, 0-N, 1-E, 2-S, 3-W, and the relative direction, this function
determines which cardinal direction to switch to. Note: This function expectes the RelativeDirection to be within the range -180 to 180 degrees*/
	UFUNCTION(BlueprintCallable, Category = "Animation Warping Utility")
	static FCardinalDirectionData GetNextCardinalDirectionEx(ECardinalDirection CurrentCardinalDirection, const float RelativeDirection, const float StepDelta = 60.0f, const float SkipDelta = 135.0f);

	UFUNCTION(BlueprintPure, Category = "Animation Utility", meta = (CompactNodeTitle = "ValidDir"))
	static bool IsValidDirection(ECardinalDirection Dir);
	UFUNCTION(BlueprintPure, Category = "Animation Utility", meta = (BlueprintThreadSafe, CompactNodeTitle = "Walk"))
	static UAnimSequence* GetWalkAnim(USoloAnimSetDefinition* AnimSet, ECardinalDirection Direction);
	UFUNCTION(BlueprintPure, Category = "Animation Utility", meta = (BlueprintThreadSafe, CompactNodeTitle = "Run"))
	static UAnimSequence* GetRunAnim(USoloAnimSetDefinition* AnimSet, ECardinalDirection Direction);
	UFUNCTION(BlueprintPure, Category = "Animation Utility")
	static float GetAnimRateScale(UAnimSequenceBase* Anim);
	
	UFUNCTION(BlueprintCallable, Category = "Animation Utility")
	static float GetAnimCurveValueAtTime(UAnimSequenceBase* Anim, FName AnimCurveName, float Time);
	UFUNCTION(BlueprintCallable, Category = "Animation Utility")
	static float FindAnimCurveTimeForValue(UAnimSequenceBase* Anim, FName AnimCurveName, float Value);

	/** Gets the rotation of an actor relative to it's velocity.*/
	UFUNCTION(BlueprintCallable, Category = "Animation Warping Utility")
	static float GetRotationRelativeToVelocityEx(const FRotator& Rotation, const FVector& Velocity);

	UFUNCTION(BlueprintCallable, Category = "Animation Utility")
	static FAnimStateMachineHandle GetStateMachineHandle(UAnimInstance* AnimInstance, FName StateMachineName);
	UFUNCTION(BlueprintCallable, Category = "Animation Utility")
	static FAnimStateMachineStateHandle GetStateMachineStateHandle(const FAnimStateMachineHandle& StateMachine, FName StateName);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Animation Utility")
	static float GetStateWeight(const FAnimStateMachineStateHandle& StateMachine);
	
	UFUNCTION(BlueprintCallable, Category = "Editor")
	static void MarkPackageDirty(UObject* Package);
	UFUNCTION(BlueprintCallable, Category = "Editor")
	static void ApplyDistanceCurveAsRootMotion(UAnimSequence* Sequence, FVector OptionalDirection = FVector::ZeroVector);
	
	static bool FindDirectionChangeTime(const UAnimSequence* Seq, float& Time);
	UFUNCTION(BlueprintCallable, Category = "Editor")
	static void GenerateSyncMarkers(UAnimSequence* Seq, bool bAnimLoops);
	UFUNCTION(BlueprintCallable, Category = "Editor")
	static void GenerateSpeedCurve(UAnimSequence* Seq, bool bAnimLoops, bool bUseFinalSpeed);
	UFUNCTION(BlueprintCallable, Category = "Editor")
	static void GenerateDistanceCurve(UAnimSequence* Seq);
	UFUNCTION(BlueprintCallable, Category = "Editor")
	static void GenerateRootMotionCurve(UAnimSequence* Seq);
	UFUNCTION(BlueprintCallable, Category = "Editor")
	static void GenerateRotationCurve(UAnimSequence* Seq);
	UFUNCTION(BlueprintCallable, Category = "Editor")
	static bool GenerateRotating(UAnimSequence* Seq, bool bDryRun);
	UFUNCTION(BlueprintCallable, Category = "Editor")
	static void GenerateDisableSpeedWarpingCurve(UAnimSequence* Seq);

	//returns true if work was done
	UFUNCTION(BlueprintCallable, Category = "Editor")
	static bool GenerateIKBonesFollowFK(UAnimSequence* Seq, bool bDryRun);

	UFUNCTION(BlueprintCallable, Category = "Editor")
	static void BuildJumpAnims(UAnimSequence* FullJump, int32 StartBeginFrame, int32 ApexBeginFrame, int32 ApexEndFrame, int32 FallEndFrame);

	static FTransform GetBonePoseForTimeRelativeToRoot(const UAnimSequence* Seq, FName Bone, float Time, bool bUseRoot = false);
};