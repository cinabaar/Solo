// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/SoloBlueprintFunctionLibrary.h"

#include "Animation/BlendSpaceBase.h"
#if WITH_EDITOR
#include "ScopedTransaction.h"
#endif
USoloAnimSetDefinition::USoloAnimSetDefinition()
{
	RunStartAnims = {{ECardinalDirection::North, nullptr}, {ECardinalDirection::East, nullptr}, {ECardinalDirection::South, nullptr}, {ECardinalDirection::West, nullptr}};
	WalkStartAnims = {{ECardinalDirection::North, nullptr}, {ECardinalDirection::East, nullptr}, {ECardinalDirection::South, nullptr}, {ECardinalDirection::West, nullptr}};
	RunStopAnims = {{ECardinalDirection::North, nullptr}, {ECardinalDirection::East, nullptr}, {ECardinalDirection::South, nullptr}, {ECardinalDirection::West, nullptr}};
	WalkStopAnims = {{ECardinalDirection::North, nullptr}, {ECardinalDirection::East, nullptr}, {ECardinalDirection::South, nullptr}, {ECardinalDirection::West, nullptr}};
	TurnAnims = {{-180, nullptr}, {-90, nullptr}, {90, nullptr}, {180, nullptr}};	
}

#if WITH_EDITOR
void USoloAnimSetDefinition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if(WalkBlend)
	{
		AnimWalkSpeed = WalkBlend->GetBlendParameter(1).Max;
	}
	if(RunBlend)
	{
		AnimRunSpeed = RunBlend->GetBlendParameter(1).Max;
	}
}
#endif

FCardinalDirectionData USoloBlueprintFunctionLibrary::GetNextCardinalDirectionEx(ECardinalDirection CurrentCardinalDirection, const float RelativeDirection, const float StepDelta, const float SkipDelta)
{
	switch (CurrentCardinalDirection)
	{
	case ECardinalDirection::Invalid:
	{
		if(RelativeDirection >= 0)
		{
			if(RelativeDirection >= 45)
			{
				if(RelativeDirection >= 90)
				{
					if(RelativeDirection >= 135)
					{
						return { ECardinalDirection::South, ECardinalDirection::East, 1 -  (RelativeDirection - 90.f) / 90.f };
					}
					else
					{
						return { ECardinalDirection::East, ECardinalDirection::South,  (RelativeDirection - 90.f) / 90.f };
					}
				}
				else
				{
					return { ECardinalDirection::East, ECardinalDirection::North, 1 - RelativeDirection / 90.f };
				}
			}
			else
			{
				return { ECardinalDirection::North, ECardinalDirection::East, RelativeDirection / 90.f };
			}
		}
		else
		{
			if (RelativeDirection < -45)
			{
				if (RelativeDirection < -90)
				{
					if (RelativeDirection < -135)
					{
						return { ECardinalDirection::South, ECardinalDirection::West, 1 - (-RelativeDirection - 90.f) / 90.f };
					}
					else
					{
						return { ECardinalDirection::West, ECardinalDirection::South,  (-RelativeDirection - 90.f) / 90.f };
					}
				}
				else
				{
					return { ECardinalDirection::West, ECardinalDirection::North, 1 - (-RelativeDirection) / 90.f };
				}
			}
			else
			{
				return { ECardinalDirection::North, ECardinalDirection::West, -RelativeDirection / 90.f };
			}
		}
	} break;
	case ECardinalDirection::North: //North
	{
		if (RelativeDirection > StepDelta)
		{
			if (RelativeDirection > SkipDelta)
			{
				return { ECardinalDirection::South, ECardinalDirection::East, 1 - (RelativeDirection - 90.f) / 90.f };
			}
			else
			{
				if(RelativeDirection <= 90.f)
				{
					return { ECardinalDirection::East, ECardinalDirection::North, 1 - RelativeDirection / 90.f };
				}
				else
				{
					return { ECardinalDirection::East, ECardinalDirection::South, (RelativeDirection - 90.f) / 90.f };
				}
			}
		}
		else if (RelativeDirection < -StepDelta)
		{
			if (RelativeDirection < -SkipDelta)
			{
				return { ECardinalDirection::South, ECardinalDirection::West, 1 - (-RelativeDirection - 90) / 90.f };
			}
			else
			{
				if(RelativeDirection >= -90.f)
				{
					return { ECardinalDirection::West, ECardinalDirection::North, 1 - (-RelativeDirection / 90.f) };
				}
				else
				{
					return {ECardinalDirection::West, ECardinalDirection::South, (-RelativeDirection - 90) / 90.f};
				}
			}
		}
		else
		{
			if(RelativeDirection >= 0)
			{
				return { ECardinalDirection::North, ECardinalDirection::East, RelativeDirection / 90.f };
			}
			else
			{
				return { ECardinalDirection::North, ECardinalDirection::West, -RelativeDirection / 90.f };
			}
		}

	} break;

	case ECardinalDirection::East: //East
	{
		float OffsetDir = RelativeDirection - 90.0f;

		if (OffsetDir < -180.0f)
			OffsetDir += 360.0f;

		if (OffsetDir > StepDelta)
		{
			if (OffsetDir > SkipDelta)
			{
				return { ECardinalDirection::West, ECardinalDirection::South, 1 - (OffsetDir - 90.f) / 90.f };
			}
			else
			{
				if(OffsetDir <= 90)
				{
					return { ECardinalDirection::South, ECardinalDirection::East,1 - OffsetDir / 90.f };
				}
				else
				{
					return { ECardinalDirection::South, ECardinalDirection::West,(OffsetDir - 90.f) / 90.f };
				}
			}
		}
		else if (OffsetDir < -StepDelta)
		{
			if (OffsetDir < -SkipDelta)
			{
				return { ECardinalDirection::West, ECardinalDirection::North, 1 - (-OffsetDir - 90) / 90.f };
			}
			else
			{
				if (OffsetDir >= -90.f)
				{
					return { ECardinalDirection::North, ECardinalDirection::East, 1 - (-OffsetDir / 90.f) };
				}
				else
				{
					return { ECardinalDirection::North, ECardinalDirection::West, (-OffsetDir - 90) / 90.f };
				}
			}
		}
		else
		{
			if (OffsetDir >= 0)
			{
				return { ECardinalDirection::East, ECardinalDirection::South, OffsetDir / 90.f };
			}
			else
			{
				return { ECardinalDirection::East, ECardinalDirection::North, -OffsetDir / 90.f };
			}
		}

	} break;

	case ECardinalDirection::South: //South
	{
		float OffsetDir = RelativeDirection - 180.0f;

		if (OffsetDir < -180.0f)
		{
			OffsetDir += 360.0f;
		}
		else if (OffsetDir > 180.0f)
		{
			OffsetDir -= 360.0f;
		}

		if (OffsetDir > StepDelta)
		{
			if (OffsetDir > SkipDelta)
			{
				return { ECardinalDirection::North, ECardinalDirection::West, 1 - (OffsetDir - 90.f) / 90.f };
			}
			else
			{
				if (OffsetDir <= 90)
				{
					return { ECardinalDirection::West, ECardinalDirection::South,1 - OffsetDir / 90.f };
				}
				else
				{
					return { ECardinalDirection::West, ECardinalDirection::North,(OffsetDir - 90.f) / 90.f };
				}
			}
		}
		else if (OffsetDir < -StepDelta)
		{
			if (OffsetDir < -SkipDelta)
			{
				return { ECardinalDirection::North, ECardinalDirection::East, 1 - (-OffsetDir - 90) / 90.f };
			}
			else
			{
				if (OffsetDir >= -90.f)
				{
					return { ECardinalDirection::East, ECardinalDirection::South, 1 - (-OffsetDir / 90.f) };
				}
				else
				{
					return { ECardinalDirection::East, ECardinalDirection::North, (-OffsetDir - 90) / 90.f };
				}
			}
		}
		else
		{
			if (OffsetDir >= 0)
			{
				return { ECardinalDirection::South, ECardinalDirection::West, OffsetDir / 90.f };
			}
			else
			{
				return { ECardinalDirection::South, ECardinalDirection::East, -OffsetDir / 90.f };
			}
		}

	} break;

	case ECardinalDirection::West: //West
	{
		float OffsetDir = RelativeDirection + 90;

		if (OffsetDir > 180.0f)
			OffsetDir -= 360.0f;

		if (OffsetDir > StepDelta)
		{
			if (OffsetDir > SkipDelta)
			{
				return { ECardinalDirection::East, ECardinalDirection::North, 1 - (OffsetDir - 90.f) / 90.f };
			}
			else
			{
				if (OffsetDir <= 90)
				{
					return { ECardinalDirection::North, ECardinalDirection::West,1 - OffsetDir / 90.f };
				}
				else
				{
					return { ECardinalDirection::North, ECardinalDirection::East,(OffsetDir - 90.f) / 90.f };
				}
			}
		}
		else if (OffsetDir < -StepDelta)
		{
			if (OffsetDir < -SkipDelta)
			{
				return { ECardinalDirection::East, ECardinalDirection::South, 1 - (-OffsetDir - 90) / 90.f };
			}
			else
			{
				if (OffsetDir >= -90.f)
				{
					return { ECardinalDirection::South, ECardinalDirection::West, 1 - (-OffsetDir / 90.f) };
				}
				else
				{
					return { ECardinalDirection::South, ECardinalDirection::East, (-OffsetDir - 90) / 90.f };
				}
			}
		}
		else
		{
			if (OffsetDir >= 0)
			{
				return { ECardinalDirection::West, ECardinalDirection::North, OffsetDir / 90.f };
			}
			else
			{
				return { ECardinalDirection::West, ECardinalDirection::South, -OffsetDir / 90.f };
			}
		}

	} break;
	default:
		check(0);
		return {};
	}
	check(0);
	return {};
}

bool USoloBlueprintFunctionLibrary::IsValidDirection(ECardinalDirection Dir)
{
	return Dir != ECardinalDirection::Invalid;
}

UAnimSequence* USoloBlueprintFunctionLibrary::GetWalkAnim(USoloAnimSetDefinition* AnimSet, ECardinalDirection Direction)
{
#if WITH_EDITOR
	if(!(AnimSet && AnimSet->WalkBlend && AnimSet->WalkBlend->IsValidBlendSampleIndex(static_cast<int32>(Direction)) && AnimSet->WalkBlend->GetBlendSample(static_cast<int32>(Direction)).Animation))
	{
		return nullptr;
	}
#endif
	return AnimSet->WalkBlend->GetBlendSample(static_cast<int32>(Direction)).Animation;
}

UAnimSequence* USoloBlueprintFunctionLibrary::GetRunAnim(USoloAnimSetDefinition* AnimSet, ECardinalDirection Direction)
{
#if WITH_EDITOR
	if(!(AnimSet && AnimSet->RunBlend && AnimSet->RunBlend->IsValidBlendSampleIndex(static_cast<int32>(Direction)) && AnimSet->RunBlend->GetBlendSample(static_cast<int32>(Direction)).Animation))
	{
		return nullptr;
	}
#endif
	return AnimSet->RunBlend->GetBlendSample(static_cast<int32>(Direction)).Animation;
}

float USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(UAnimSequenceBase* Anim, FName AnimCurveName, float Time)
{
	if(!Anim)
	{
		return 0.f;
	}
	FSmartName CurveSmartName;
	Anim->GetSkeleton()->GetSmartNameByName(USkeleton::AnimCurveMappingName, AnimCurveName, CurveSmartName);
	if(!CurveSmartName.IsValid())
	{
		return 0.f;
	}
	const FFloatCurve* AnimCurve = static_cast<const FFloatCurve*>(Anim->GetCurveData().GetCurveData(CurveSmartName.UID));
	if(!AnimCurve)
	{
		return  0.f;
	}
	return AnimCurve->Evaluate(Time);
}

float USoloBlueprintFunctionLibrary::FindAnimCurveTimeForValue(UAnimSequenceBase* Anim, FName AnimCurveName, float Value)
{
	if (!Anim)
	{
		return 0.f;
	}
	FSmartName CurveSmartName;
	Anim->GetSkeleton()->GetSmartNameByName(USkeleton::AnimCurveMappingName, AnimCurveName, CurveSmartName);
	if (!CurveSmartName.IsValid())
	{
		return 0.f;
	}
	const FFloatCurve* AnimCurve = static_cast<const FFloatCurve*>(Anim->GetCurveData().GetCurveData(CurveSmartName.UID));
	if (!AnimCurve)
	{
		return  0.f;
	}
	float StartSearchPos = 0.f, EndSearchPos = Anim->SequenceLength;
	float CurrentSearchPos = 0.f;
	float EvalValue = 0.f;
	while(StartSearchPos <= EndSearchPos - KINDA_SMALL_NUMBER)
	{
		CurrentSearchPos = (EndSearchPos + StartSearchPos) / 2.f;
		EvalValue = AnimCurve->Evaluate(CurrentSearchPos);
		if(EvalValue > Value)
		{
			EndSearchPos = CurrentSearchPos;
		}
		else
		{
			StartSearchPos = CurrentSearchPos;
		}
	}
	return FMath::Clamp(CurrentSearchPos, 0.f, Anim->SequenceLength);
}

float USoloBlueprintFunctionLibrary::GetRotationRelativeToVelocityEx(const FRotator& Rotation, const FVector& Velocity)
{
	if (Velocity.SizeSquared() < 0.0001f)
		return 0.0f;
	
	FRotator Orientation = Velocity.ToOrientationRotator();
	
	float RotationZ = (Rotation - Orientation).Yaw * -1.0f;

	if (RotationZ > 180.0f)
	{
		RotationZ -= 360.0f;
	}
	else if (RotationZ < -180.0f)
	{
		RotationZ += 360.0f;
	}

	return RotationZ;
}

void USoloBlueprintFunctionLibrary::MarkPackageDirty(UObject* Package)
{
#if WITH_EDITOR
	if(Package)
	{
		Package->MarkPackageDirty();
	}
#endif
}

void USoloBlueprintFunctionLibrary::ApplyDistanceCurveAsRootMotion(UAnimSequence* Sequence, FVector OptionalDirection)
{
#if WITH_EDITOR
	if(!Sequence)
	{
		return;
	}
	FSmartName CurveSmartName;
	Sequence->GetSkeleton()->GetSmartNameByName(USkeleton::AnimCurveMappingName, TEXT("DistanceCurve"), CurveSmartName);
	if(!CurveSmartName.IsValid())
	{
		return;
	}
	const FFloatCurve* AnimCurve = static_cast<const FFloatCurve*>(Sequence->GetCurveData().GetCurveData(CurveSmartName.UID));
	if(!AnimCurve)
	{
		return;
	}
	const FScopedTransaction Transaction(FText::FromString(TEXT("ApplyDistanceCurveAsRootMotion")));
	//Call modify to restore anim sequence current state
	Sequence->Modify();

	FVector Direction;
	if(OptionalDirection.IsNearlyZero())
	{
		FRawAnimSequenceTrack& RawTrack = Sequence->GetRawAnimationTrack(0);
		Direction = RawTrack.PosKeys.Last() - RawTrack.PosKeys[0];
	}
	else
	{
		Direction = OptionalDirection;
	}
	Direction.Normalize();
	FRawAnimSequenceTrack& RawTrack = Sequence->GetRawAnimationTrack(0);
	RawTrack.PosKeys.Empty();
	for(int32 i = 0; i < Sequence->GetNumberOfFrames(); ++i)
	{
		auto DistanceCurveValue = AnimCurve->Evaluate(Sequence->GetTimeAtFrame(i));
		RawTrack.PosKeys.Add(Direction * DistanceCurveValue);
	}
	Sequence->MarkRawDataAsModified();
	Sequence->OnRawDataChanged();

	Sequence->MarkPackageDirty();
#endif
}
