// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SoloBlueprintFunctionLibrary.generated.h"

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

class UBlendSpaceBase;

UCLASS(BlueprintType)
class SOLO_API USoloAnimSetDefinition : public UDataAsset
{
	GENERATED_BODY()
public:
	USoloAnimSetDefinition();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UBlendSpaceBase* RunBlend = nullptr;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float AnimRunSpeed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UBlendSpaceBase* WalkBlend = nullptr;
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
	float AnimWalkSpeed = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize))
	TMap<ECardinalDirection, UAnimSequence*> RunStartAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize))
	TMap<ECardinalDirection, UAnimSequence*> WalkStartAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize))
	TMap<ECardinalDirection, UAnimSequence*> RunStopAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize))
	TMap<ECardinalDirection, UAnimSequence*> WalkStopAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (EditFixedSize))
	TMap<int32, UAnimSequence*> TurnAnims;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimSequence* JumpAnim = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimSequence* JumpApex = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UAnimSequence* JumpPreLand = nullptr;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};

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
	
	UFUNCTION(BlueprintCallable, Category = "Animation Utility")
	static float GetAnimCurveValueAtTime(UAnimSequenceBase* Anim, FName AnimCurveName, float Time);
	UFUNCTION(BlueprintCallable, Category = "Animation Utility")
	static float FindAnimCurveTimeForValue(UAnimSequenceBase* Anim, FName AnimCurveName, float Value);

	/** Gets the rotation of an actor relative to it's velocity.*/
	UFUNCTION(BlueprintCallable, Category = "Animation Warping Utility")
	static float GetRotationRelativeToVelocityEx(const FRotator& Rotation, const FVector& Velocity);

	UFUNCTION(BlueprintCallable, Category = "Editor")
	static void MarkPackageDirty(UObject* Package);
	UFUNCTION(BlueprintCallable, Category = "Editor")
	static void ApplyDistanceCurveAsRootMotion(UAnimSequence* Sequence, FVector OptionalDirection);
};
