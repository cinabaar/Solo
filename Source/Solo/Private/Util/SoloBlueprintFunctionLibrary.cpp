// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/SoloBlueprintFunctionLibrary.h"

#include "Algo/MaxElement.h"
#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNode_StateMachine.h"
#include "Animation/AnimStateMachineTypes.h"
#include "Animation/BlendSpace.h"
#include "Internationalization/Regex.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimSequenceHelpers.h"

#if WITH_EDITOR
#include "AnimationBlueprintLibrary/Public/AnimationBlueprintLibrary.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EditorAssetLibrary.h"
#endif
FString EnumToString(const ECardinalDirection Value)
{
	static const UEnum* TypeEnum = FindObject<UEnum>(ANY_PACKAGE, TEXT("ECardinalDirection"));
	return TypeEnum->GetNameStringByIndex(static_cast<int32>(Value));
}

USoloAnimSetDefinition::USoloAnimSetDefinition()
{
	RunStartAnims = WalkStartAnims = RunStopAnims = WalkStopAnims = WalkAnims = RunAnims = SprintStartAnims = SprintStopAnims = SprintAnims = /*RunRotStartAnims = WalkRotStartAnims = SprintRotStartAnims =*/
		{{ECardinalDirection::North, nullptr}, {ECardinalDirection::East, nullptr}, {ECardinalDirection::South, nullptr}, {ECardinalDirection::West, nullptr}};
	TurnAnims = {{-180, nullptr}, {-90, nullptr}, {90, nullptr}, {180, nullptr}};	
}

#if WITH_EDITOR
void USoloAnimSetDefinition::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	auto Fix = [](TMap<ECardinalDirection, UAnimSequence*>& Map)
	{
		Map.FindOrAdd(ECardinalDirection::North);
		Map.FindOrAdd(ECardinalDirection::East);
		Map.FindOrAdd(ECardinalDirection::South);
		Map.FindOrAdd(ECardinalDirection::West);
	};
	Fix(RunStartAnims);
	Fix(WalkStartAnims);
	Fix(RunStopAnims);
	Fix(WalkStopAnims);
	Fix(WalkAnims);
	Fix(RunAnims);
	Fix(SprintStartAnims);
	Fix(SprintStopAnims);
	Fix(SprintAnims);
	// Fix(WalkRotStartAnims);
	// Fix(RunRotStartAnims);
	// Fix(SprintRotStartAnims);
	TurnAnims.FindOrAdd(-180);
	TurnAnims.FindOrAdd(-90);
	TurnAnims.FindOrAdd(90);
	TurnAnims.FindOrAdd(180);
}

void FAnimSetCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);
	if(Objects.Num() != 1)
		return;
	auto* AnimSet = Cast<USoloAnimSetDefinition>(Objects[0]);
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory(TEXT("Validations"), FText::GetEmpty(), ECategoryPriority::Variable);
	TSharedPtr<SVerticalBox> VerticalBox = SNew(SVerticalBox);

	auto AddRow = [&DetailBuilder, VerticalBox](UAnimationAsset* Seq, FString Text, bool bFix, TFunction<void()> FixFunc)
	{
		VerticalBox->AddSlot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(STextBlock).Text(FText::FromString(Text))
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[ SNew(SSpacer).Size(FVector2D(50.f ,1.f))]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton).Text(FText::FromString(bFix? TEXT("Fix") : Seq? TEXT("Open") : TEXT("Refresh")))
				.OnClicked_Lambda([&DetailBuilder, FixFunc, bFix, Seq]()
				{
					if(bFix)
					{
						FixFunc();
					}
					else if(Seq)
					{
						GEditor->GetEditorSubsystem<UAssetEditorSubsystem>()->OpenEditorForAsset(Seq);
					}
					DetailBuilder.ForceRefreshDetails();
					return FReply::Handled();
				})
			]
		];	
	};

	auto CopyAnims = [AnimSet](bool bDryRun) -> bool
	{
		FString Path = AnimSet->GetPathName();
		Path.Split(TEXT("."), &Path, nullptr);
		Path += "/";

		auto ProcessAnim = [&](UAnimSequence*& Anim, const FString& NewAnimName) -> bool
		{
			if(!Anim)
			{
				return false;
			}
			FString AnimPath = Anim->GetPathName();
			FString AnimName;
			AnimPath.Split(TEXT("."), &AnimPath, &AnimName);
			if((!AnimPath.Contains(Path) || AnimName != NewAnimName) && !(NewAnimName.Contains(TEXT("Stop")) && Anim == AnimSet->Idle))
			{
				if(!bDryRun)
				{
					if(auto* NewAnim = UEditorAssetLibrary::DuplicateAsset(AnimPath, Path + NewAnimName))
					{
						Anim = Cast<UAnimSequence>(NewAnim);
						AnimSet->MarkPackageDirty();
					}
				}
				return true;
			}
			return false;
		};
		auto DirToName = [](ECardinalDirection Dir) -> FString
		{
			switch(Dir)
			{
			case ECardinalDirection::North: return TEXT("Fwd");
			case ECardinalDirection::East: return TEXT("Right");
			case ECardinalDirection::South: return TEXT("Bwd");
			case ECardinalDirection::West: return TEXT("Left");
			default: return TEXT("Invalid");
			}
		};
		bool bResult = false;

		bResult |= ProcessAnim(AnimSet->Idle, TEXT("Idle"));
		{
			if(AnimSet->AimOffset)
			{
				FString AnimPath = AnimSet->AimOffset->GetPathName();
				FString AnimName;
				AnimPath.Split(TEXT("."), &AnimPath, &AnimName);
				FString NewAnimName = TEXT("AO_Idle");
				bool bAllSamplesValid = true;

				const TArray<FBlendSample>& BlendSamples = AnimSet->AimOffset->GetBlendSamples();

				auto GetSampleName = [](const FVector& SampleValue) -> FString
				{
					FString BaseName = TEXT("AO_Idle_");
					if(SampleValue.Y == 0)
					{
						BaseName += TEXT("C");
					}
					else if(SampleValue.Y > 0)
					{
						BaseName += TEXT("U");
					}
					else
					{
						BaseName += TEXT("D");
					}
					if(SampleValue.X == 0)
					{
						BaseName += TEXT("C");
					}
					else if(SampleValue.X > 0)
					{
						BaseName += TEXT("R");
					}
					else
					{
						BaseName += TEXT("L");
					}
					return BaseName;
				};
				for(auto& Sample : BlendSamples)
				{
					FString SampleAnimPath = Sample.Animation->GetPathName();
					FString SampleAnimName;
					SampleAnimPath.Split(TEXT("."), &SampleAnimPath, &SampleAnimName);
					if(!SampleAnimPath.Contains(Path) || SampleAnimName != GetSampleName(Sample.SampleValue))
					{
						bAllSamplesValid = false;
						break;
					}
				}
				if(!AnimPath.Contains(Path) || AnimName != TEXT("AO_Idle") || !bAllSamplesValid)
				{
					if(!bDryRun)
					{
						if(auto* NewAnim = UEditorAssetLibrary::DuplicateAsset(AnimPath, Path + NewAnimName))
						{
							AnimSet->AimOffset = Cast<UAimOffsetBlendSpace>(NewAnim);
							TArray<TTuple<FVector, UAnimSequence*>> NewSamples;
							for(int32 i = 0; i < BlendSamples.Num(); ++i)
							{
								auto& Sample = BlendSamples[i];
								FString SampleAnimPath = Sample.Animation->GetPathName();
								FString SampleAnimName;
								SampleAnimPath.Split(TEXT("."), &SampleAnimPath, &SampleAnimName);
								auto* NewSample = UEditorAssetLibrary::DuplicateAsset(SampleAnimPath, Path + TEXT("AO_Idle/") + GetSampleName(Sample.SampleValue));
								NewSamples.Add(MakeTuple(Sample.SampleValue, Cast<UAnimSequence>(NewSample)));
							}
							for(int32 i = BlendSamples.Num() - 1; i >= 0; --i)
							{
								AnimSet->AimOffset->DeleteSample(i);
							}
							for(auto& Sample : NewSamples)
							{
								AnimSet->AimOffset->AddSample(Sample.Value, Sample.Key);
							}
							AnimSet->MarkPackageDirty();
						}
					}
					bResult |= true;
				}
			}
		}
		for(TFieldIterator<FMapProperty> It(AnimSet->GetClass()); It; ++It)
		{
			FScriptMapHelper_InContainer MapHelper{*It, AnimSet};
			FString NameFormat = It->GetMetaData(TEXT("NameFormat"));
			
			if(It->KeyProp->IsA<FEnumProperty>())
			{
				for(auto MapIt = MapHelper.CreateIterator(); MapIt; ++MapIt)
				{
					uint8* PairPtr = MapHelper.GetPairPtr(*MapIt);
					const ECardinalDirection* Key = MapHelper.GetKeyProperty()->ContainerPtrToValuePtr<ECardinalDirection>(PairPtr);
					UAnimSequence** Value = MapHelper.GetValueProperty()->ContainerPtrToValuePtr<UAnimSequence*>(PairPtr);
					FString FormattedName = FString::Format(*NameFormat, FStringFormatNamedArguments{{TEXT("Dir"), DirToName(*Key)}});
					if(Value)
					{
						bResult |= ProcessAnim(*Value, FormattedName);
					}
				}
			}
		}
		for(auto It = AnimSet->TurnAnims.CreateIterator(); It; ++It)
		{
			FString Dir = It->Key > 0 ? TEXT("Right") : TEXT("Left");
			FString Amount = FString::FromInt(FMath::Abs(It->Key));
			bResult |= ProcessAnim(It->Value, TEXT("Turn_") + Dir + "_" + Amount);
		}
		return bResult;
	};

	auto AddCopyAnimsRow = [&DetailBuilder, VerticalBox, CopyAnims]()
	{
		VerticalBox->AddSlot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(STextBlock).Text(FText::FromString(TEXT("You are using animations for which the naming/location does not comply with conventions")))
			]
			+ SHorizontalBox::Slot().AutoWidth()
			[ SNew(SSpacer).Size(FVector2D(50.f ,1.f))]
			+ SHorizontalBox::Slot().AutoWidth()
			[
				SNew(SButton).Text(FText::FromString(TEXT("Fix")))
				.OnClicked_Lambda([&DetailBuilder, CopyAnims]()
				{
					CopyAnims(false);
					DetailBuilder.ForceRefreshDetails();
					return FReply::Handled();
				})
			]
		];	
	};

	auto ValidateSyncMarkers = [=](UAnimSequence* Seq, bool bLooping)
	{
		int32 RSync = 0, LSync = 0;
		float LastSyncMarkerTime = 0.f;
		Seq->SortSyncMarkers();
		bool bRepeatingSyncMarkers = false;
		int32 Last = INDEX_NONE;
		for(auto& Sync : Seq->AuthoredSyncMarkers)
		{
			if(Sync.Time > LastSyncMarkerTime)
			{
				LastSyncMarkerTime = Sync.Time;
			}
			if(Sync.MarkerName == TEXT("foot_r"))
			{
				RSync++;
				if(Last == 0)
				{
					bRepeatingSyncMarkers = true;
				}
				Last = 0;
			}
			else if(Sync.MarkerName == TEXT("foot_l"))
			{
				LSync++;
				if(Last == 1)
				{
					bRepeatingSyncMarkers = true;
				}
				Last = 1;
			}
		}
		float TimeBetweenSyncMarkers = Seq->GetDataModel()->GetPlayLength();
		for(int i = 0; i < Seq->AuthoredSyncMarkers.Num() - 1; ++i)
		{
			const float TimeBetween = (Seq->AuthoredSyncMarkers[i+1].Time - Seq->AuthoredSyncMarkers[i].Time);
			if(TimeBetween < TimeBetweenSyncMarkers)
			{
				TimeBetweenSyncMarkers = TimeBetween;
			}
		}

		if(RSync == 0 && LSync == 0)
		{
			AddRow(Seq, FString::Printf(TEXT("Invalid sync marker setup in %s. Try to fix?"), *Seq->GetName()), true, [=]()
			{
				USoloBlueprintFunctionLibrary::GenerateSyncMarkers(Seq, bLooping);
			});
		}
		else if(!bLooping && LastSyncMarkerTime < Seq->GetDataModel()->GetPlayLength() * 0.8f)
		{
			AddRow(Seq, FString::Printf(TEXT("Sync marker too far from end in non-looping animation %s. Won't work too well"), *Seq->GetName()), false, [=](){});
		}
		else if(RSync != LSync || bRepeatingSyncMarkers)
		{
			AddRow(Seq, FString::Printf(TEXT("Uneven amount of sync markers in %s. Please fix"), *Seq->GetName()), false, [=](){});
		}
		else if(TimeBetweenSyncMarkers < Seq->GetDataModel()->GetPlayLength() * 0.10f)
		{
			AddRow(Seq, FString::Printf(TEXT("Sync markers seem to be too close to each other in %s. Please fix"), *Seq->GetName()), false, [=](){});
		}
	};

	auto ValidateExists = [=](UAnimationAsset* Seq, FString Type, FString Id)
	{
		if(!Seq || !Seq->GetSkeleton())
		{
			AddRow(Seq, FString::Printf(TEXT("Missing %s anim %s"), *Type, *Id), false, [](){});
			return false;
		}
		return true;
	};
	auto ValidateHasCurve = [=](UAnimSequence* Seq, FString Type, FName CurveName, bool bLooping = false, bool bNonZero = true)
	{
		const auto* Skeleton = Seq->GetSkeleton();
		FSmartName CurveSmartName;
		const bool bCurveExists = Skeleton->GetSmartNameByName(USkeleton::AnimCurveMappingName, CurveName, CurveSmartName);
		const float StartCurveValue = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(Seq, CurveName, 0);
		const float EndCurveValue = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(Seq, CurveName, Seq->GetDataModel()->GetPlayLength());
		const bool bStartAnim = Type.Contains(TEXT("Start"));
		const bool bShouldHaveEvenSpeedAtEnds = CurveName == TEXT("Speed") && (bStartAnim || bLooping);
		if(!bCurveExists || !Seq->HasCurveData(CurveSmartName.UID, false) || (bShouldHaveEvenSpeedAtEnds && StartCurveValue != EndCurveValue) || (bShouldHaveEvenSpeedAtEnds && (StartCurveValue == 0.f || FMath::Abs(StartCurveValue) > 1000.f)))
		{
			AddRow(Seq, FString::Printf(TEXT("Missing %s curve for %s anim %s"), *CurveName.ToString(), *Type, *Seq->GetName()), true, [=]()
			{
				if(CurveName == TEXT("Speed"))
				{
					USoloBlueprintFunctionLibrary::GenerateSpeedCurve(Seq, bLooping, bStartAnim);
				}
				else if(CurveName == TEXT("DistanceCurve"))
				{
					USoloBlueprintFunctionLibrary::GenerateDistanceCurve(Seq);
				}
				else if(CurveName == TEXT("DisableSpeedWarping"))
				{
					USoloBlueprintFunctionLibrary::GenerateDisableSpeedWarpingCurve(Seq);
				}
				else if(CurveName == TEXT("RotationCurve"))
				{
					USoloBlueprintFunctionLibrary::GenerateRotationCurve(Seq);
				}
				else if(CurveName == TEXT("bRotating"))
				{
					USoloBlueprintFunctionLibrary::GenerateRotating(Seq, false);
				}
			});
		}
	};
	auto ValidateIKBones = [=](UAnimSequence* Seq)
	{
		if(USoloBlueprintFunctionLibrary::GenerateIKBonesFollowFK(Seq, true))
		{
			AddRow(Seq, FString::Printf(TEXT("IK bones not alligned in anim %s"), *Seq->GetName()), true, [Seq]()
			{
				USoloBlueprintFunctionLibrary::GenerateIKBonesFollowFK(Seq, false);
			});
		}
		return true;
	};
	auto ValidateRootNotMoving = [=](UAnimSequence* Seq)
	{
		bool bRootMoving = false;
		for(int32 i = 0; i < Seq->GetDataModel()->GetNumberOfFrames(); ++i)
		{
			FTransform FramePose;
			UAnimationBlueprintLibrary::GetBonePoseForFrame(Seq, TEXT("root"), i, true, FramePose);

			if(FramePose.GetLocation() != FVector::ZeroVector && FramePose.Rotator() != FRotator::ZeroRotator)
			{
				bRootMoving = true;
				break;
			}
		}
		if(bRootMoving && !Seq->bForceRootLock)
		{
			AddRow(Seq, FString::Printf(TEXT("Anim %s has root motion. Lock root motion."), *Seq->GetName()), true, [Seq]()
			{
				Seq->bForceRootLock = true;
			});
		}
		return true;
	};

	auto ValidateAnims = [=](TMap<ECardinalDirection, UAnimSequence*> Anims, FString Type, TArray<FName> Curves, bool bSyncMarkers, bool bLooping = false)
	{
		for(const auto& Anim : Anims)
		{
			if(!ValidateExists(Anim.Value, Type, EnumToString(Anim.Key)))
			{
				continue;
			}
			for(const auto& Curve : Curves)
			{
				ValidateHasCurve(Anim.Value, Type, Curve, bLooping);
			}
			if(bSyncMarkers)
			{
				ValidateSyncMarkers(Anim.Value, bLooping);
			}
			ValidateIKBones(Anim.Value);
			ValidateRootNotMoving(Anim.Value);
		}
	};
	auto ValidateAO = [=](UAimOffsetBlendSpace* AO)
	{
		ValidateExists(AO, TEXT("AimOffset"), TEXT(""));
		if(AO)
		{
			const bool bValidPreview = AO->PreviewBasePose == AnimSet->Idle;
			if(!bValidPreview)
			{
				AddRow(AO, FString::Printf(TEXT("Anim %s invalid preview base pose."), *AO->GetName()), true, [=]()
				{
					AO->PreviewBasePose = AnimSet->Idle;
					AO->MarkPackageDirty();
				});
			}
			const bool bValidSamples = AO->GetBlendSamples().Num() > 0;
			if(!bValidSamples)
			{
				AddRow(AO, FString::Printf(TEXT("Anim %s doesn't have any samples defined"), *AO->GetName()), false, [](){});
			}
			for(auto& Sample : AO->GetBlendSamples())
			{
				if(Sample.Animation->AdditiveAnimType != AAT_RotationOffsetMeshSpace || Sample.Animation->RefPoseSeq != AnimSet->Idle  || Sample.Animation->RefPoseType != ABPT_AnimFrame)
				{
					AddRow(Sample.Animation, FString::Printf(TEXT("Anim %s has invalid aim offset setup."), *Sample.Animation->GetName()), true, [=]()
					{
						Sample.Animation->AdditiveAnimType = AAT_RotationOffsetMeshSpace;
						Sample.Animation->RefPoseSeq = AnimSet->Idle;
						Sample.Animation->RefPoseType = ABPT_AnimFrame;
						Sample.Animation->MarkPackageDirty();
					});
				}
			}
		}
	};
	AddRow(nullptr, TEXT(""), false, [](){});
	if(CopyAnims(true))
	{
		AddCopyAnimsRow();
	}
	ValidateExists(AnimSet->Idle, TEXT("Idle"), TEXT(""));
	ValidateAO(AnimSet->AimOffset);
	ValidateAnims(AnimSet->WalkAnims, TEXT("Walk"), {TEXT("Speed")}, true, true);
	ValidateAnims(AnimSet->RunAnims, TEXT("Run"), {TEXT("Speed")}, true, true);
	ValidateAnims(AnimSet->SprintAnims, TEXT("Sprint"), {TEXT("Speed")}, true, true);
	ValidateAnims(AnimSet->WalkStartAnims, TEXT("WalkStart"), {TEXT("Speed"), TEXT("DistanceCurve"), TEXT("DisableSpeedWarping")}, true);
	ValidateAnims(AnimSet->RunStartAnims, TEXT("RunStart"), {TEXT("Speed"), TEXT("DistanceCurve"), TEXT("DisableSpeedWarping")}, true);
	ValidateAnims(AnimSet->SprintStartAnims, TEXT("SprintStart"), {TEXT("Speed"), TEXT("DistanceCurve"), TEXT("DisableSpeedWarping")}, true);
	ValidateAnims(AnimSet->WalkStopAnims, TEXT("WalkStop"), {TEXT("Speed"), TEXT("DistanceCurve")}, false);
	ValidateAnims(AnimSet->RunStopAnims, TEXT("RunStop"), {TEXT("Speed"), TEXT("DistanceCurve")}, false);
	ValidateAnims(AnimSet->SprintStopAnims, TEXT("SprintStop"), {TEXT("Speed"), TEXT("DistanceCurve")}, false);
	// ValidateAnims(AnimSet->WalkRotStartAnims, TEXT("WalkRot"), {TEXT("Speed"), TEXT("DistanceCurve"), TEXT("DisableSpeedWarping"), TEXT("RotationCurve")}, true);
	// ValidateAnims(AnimSet->RunRotStartAnims, TEXT("RunRot"), {TEXT("Speed"), TEXT("DistanceCurve"), TEXT("DisableSpeedWarping"), TEXT("RotationCurve")}, true);
	// ValidateAnims(AnimSet->SprintRotStartAnims, TEXT("SprintRot"), {TEXT("Speed"), TEXT("DistanceCurve"), TEXT("DisableSpeedWarping"), TEXT("RotationCurve")}, true);

	const FRegexPattern TurnAnimNamePattern(TEXT(".*_(Left|Right|L|R)_(90|180).*"));
	for(const auto& Anim : AnimSet->TurnAnims)
	{
		if(!ValidateExists(Anim.Value, TEXT("Turn"), FString::FromInt(Anim.Key)))
		{
			continue;
		}

		ValidateIKBones(Anim.Value);
		ValidateRootNotMoving(Anim.Value);
		if(FRegexMatcher Matcher(TurnAnimNamePattern, Anim.Value->GetName()); !Matcher.FindNext())
		{
			AddRow(Anim.Value, TEXT("Turn anims should have [Left|Right|L|R]_[90|180] with possible prefixes and suffixes"), false, [](){});
			continue;
		}
		ValidateHasCurve(Anim.Value, TEXT("Turn"), TEXT("RotationCurve"));
		ValidateHasCurve(Anim.Value, TEXT("Turn"), TEXT("bRotating"));
	}
	
	Category.AddCustomRow(FText::FromString(TEXT("Validations"))).WholeRowContent()
	[
		VerticalBox->AsShared()
	];

	TSharedPtr<SVerticalBox> CurveControlBox = SNew(SVerticalBox);

	auto DisplayCurveControls = [&DetailBuilder, CurveControlBox](UAnimSequence* Seq)
	{
		const auto HorizontalBox = SNew(SHorizontalBox);
		auto* SmartNameContainer = Seq->GetSkeleton()->GetSmartNameContainer(USkeleton::AnimCurveMappingName);
		TArray<FName> SmartNames;
		SmartNames.Sort(FNameLexicalLess());
		SmartNameContainer->FillNameArray(SmartNames);
		HorizontalBox->AddSlot().AutoWidth()
		[
			SNew(SBox).MaxDesiredHeight(500).MinDesiredWidth(500)
			[
				SNew(STextBlock).Text(FText::FromString(Seq->GetName()))
			]
		];
		for(auto& Name : SmartNames)
		{
			FSmartName CurveSmartName;
			Seq->GetSkeleton()->GetSmartNameByName(USkeleton::AnimCurveMappingName, Name, CurveSmartName);
			if(const FFloatCurve* AnimCurve = static_cast<const FFloatCurve*>(Seq->GetCurveData().GetCurveData(CurveSmartName.UID)); !AnimCurve)
			{
				continue;
			}
			auto Slot = HorizontalBox->AddSlot();
			Slot.AutoWidth()
			[
				SNew(SButton).Text(FText::FromName(Name)).OnClicked_Lambda([&DetailBuilder, Name, Seq]()
				{
					UAnimationBlueprintLibrary::RemoveCurve(Seq, Name);
					Seq->MarkPackageDirty();
					DetailBuilder.ForceRefreshDetails();
					return FReply::Handled();
				})
			];
		}
		CurveControlBox->AddSlot()
		[
			HorizontalBox->AsShared()
		];
	};
	
	for(TFieldIterator<FProperty> It(AnimSet->GetClass()); It; ++It)
	{
		if(const FObjectProperty* ObjProp = CastField<FObjectProperty>(*It))
		{
			UObject* Object = ObjProp->GetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(AnimSet, 0));
			if(UAnimSequence* Seq = Cast<UAnimSequence>(Object))
			{
				DisplayCurveControls(Seq);
			}
		}
		else if(FMapProperty* MapProp = CastField<FMapProperty>(*It))
		{
			FScriptMapHelper_InContainer MapHelper{MapProp, AnimSet};

			for(auto MapIt = MapHelper.CreateIterator(); MapIt; ++MapIt)
			{
				uint8* PairPtr = MapHelper.GetPairPtr(*MapIt);
				UAnimSequence** Value = MapHelper.GetValueProperty()->ContainerPtrToValuePtr<UAnimSequence*>(PairPtr);
				if(Value && *Value)
				{
					DisplayCurveControls(*Value);
				}
			}
		}
	}
	IDetailCategoryBuilder& CurveControlCategory = DetailBuilder.EditCategory(TEXT("CurveControl"), FText::GetEmpty(), ECategoryPriority::Important);
	CurveControlCategory.AddCustomRow(FText::FromString(TEXT("CurveControl"))).WholeRowContent()
	[
		CurveControlBox->AsShared()
	];
}

TSharedRef<IDetailCustomization> FAnimSetCustomization::MakeInstance()
{
	return MakeShareable(new FAnimSetCustomization());
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

bool USoloBlueprintFunctionLibrary::IsValidDirection(const ECardinalDirection Dir)
{
	return Dir != ECardinalDirection::Invalid;
}

UAnimSequence* USoloBlueprintFunctionLibrary::GetWalkAnim(USoloAnimSetDefinition* AnimSet, ECardinalDirection Direction)
{
	return AnimSet->WalkAnims[Direction];
}

UAnimSequence* USoloBlueprintFunctionLibrary::GetRunAnim(USoloAnimSetDefinition* AnimSet, ECardinalDirection Direction)
{
	return AnimSet->RunAnims[Direction];
}

float USoloBlueprintFunctionLibrary::GetAnimRateScale(UAnimSequenceBase* Anim)
{
	return Anim? Anim->RateScale : 0.f;
}

float USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(UAnimSequenceBase* Anim, const FName AnimCurveName, float Time)
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
	float StartSearchPos = 0.f, EndSearchPos = Anim->GetDataModel()->GetPlayLength();
	float CurrentSearchPos = 0.f;
	while(StartSearchPos <= EndSearchPos - KINDA_SMALL_NUMBER)
	{
		CurrentSearchPos = (EndSearchPos + StartSearchPos) / 2.f;
		if(const float EvalValue = AnimCurve->Evaluate(CurrentSearchPos); EvalValue >= Value)
		{
			EndSearchPos = CurrentSearchPos;
		}
		else
		{
			StartSearchPos = CurrentSearchPos;
		}
	}
	return FMath::Clamp(CurrentSearchPos, 0.f, Anim->GetDataModel()->GetPlayLength());
}

float USoloBlueprintFunctionLibrary::GetRotationRelativeToVelocityEx(const FRotator& Rotation, const FVector& Velocity)
{
	if (Velocity.SizeSquared() < 0.0001f)
		return 0.0f;

	const FRotator Orientation = Velocity.ToOrientationRotator();
	
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

FAnimStateMachineHandle USoloBlueprintFunctionLibrary::GetStateMachineHandle(UAnimInstance* AnimInstance, FName StateMachineName)
{
	FAnimStateMachineHandle Result;
	auto* StateMachineDesc = AnimInstance->GetStateMachineInstanceDesc(StateMachineName);
	Result.StateMachineDesc = StateMachineDesc;
	Result.StateMachine = AnimInstance->GetStateMachineInstanceFromName(StateMachineName);
	return Result;
}

FAnimStateMachineStateHandle USoloBlueprintFunctionLibrary::GetStateMachineStateHandle(const FAnimStateMachineHandle& StateMachine, FName StateName)
{
	FAnimStateMachineStateHandle Result;
	if(!StateMachine.StateMachine || !StateMachine.StateMachineDesc)
	{
		return Result;
	}
	Result.StateMachine = StateMachine.StateMachine;
	Result.StateIndex = StateMachine.StateMachineDesc->FindStateIndex(StateName);
	return Result;
}

float USoloBlueprintFunctionLibrary::GetStateWeight(const FAnimStateMachineStateHandle& StateMachine)
{
	if(!StateMachine.StateMachine)
	{
		return 0.f;
	}
	return StateMachine.StateMachine->GetStateWeight(StateMachine.StateIndex);
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

bool USoloBlueprintFunctionLibrary::FindDirectionChangeTime(const UAnimSequence* Seq, float& Time)
{
	TArray<float> DistanceCurveValues;
	DistanceCurveValues.Reserve(Seq->GetDataModel()->GetNumberOfFrames());
	for(int32 i = 0; i < Seq->GetDataModel()->GetNumberOfFrames(); ++i)
	{
		FTransform RootPose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), Seq->GetTimeAtFrame(i), true, RootPose);
		float Distance = RootPose.GetLocation().Size();
		Distance = static_cast<int32>(Distance * 10.f) / 10.f;
		DistanceCurveValues.Add(Distance);
	}

	bool bLastWasReversed = false;
	bool bFoundChange = false;
	for(int i = 0; i < DistanceCurveValues.Num() - 1; ++i)
	{
		float& CurrentValue = DistanceCurveValues[i];
		if(const float& NextValue = DistanceCurveValues[i+1]; NextValue < CurrentValue)
		{
			CurrentValue *= -1;
			bLastWasReversed = true;
		}
		else if(bLastWasReversed && CurrentValue > KINDA_SMALL_NUMBER)
		{
			const float& PrevValue = DistanceCurveValues[i-1];
			bLastWasReversed = false;
			bFoundChange = true;
			CurrentValue = (PrevValue + NextValue) / 2.f;
			Time = Seq->GetTimeAtFrame(i+1) - (NextValue) / (NextValue - PrevValue);
		}
	}
	
	return bFoundChange;
}

void USoloBlueprintFunctionLibrary::GenerateSyncMarkers(UAnimSequence* Seq, bool bAnimLoops)
{
#if WITH_EDITOR

	const FName PelvisBonsName = TEXT("pelvis");
	const FName NotifyTrackName = TEXT("SyncMarkers");

	UAnimationBlueprintLibrary::RemoveAnimationNotifyTrack(Seq, NotifyTrackName);
	UAnimationBlueprintLibrary::AddAnimationNotifyTrack(Seq, NotifyTrackName);

	auto PlaceSyncMarkersBetweenTimes = [&](const float CurrentTime, const float NextTime, const FName FootBoneName, const FVector Dir)
	{
		const FVector CurrentFootLoc = GetBonePoseForTimeRelativeToRoot(Seq, FootBoneName, CurrentTime).GetLocation();
		const FVector NextFootLoc= GetBonePoseForTimeRelativeToRoot(Seq, FootBoneName, NextTime).GetLocation();
		const FVector CurrentPelvisLoc = GetBonePoseForTimeRelativeToRoot(Seq, PelvisBonsName, CurrentTime).GetLocation();
		const FVector NextPelvisLoc = GetBonePoseForTimeRelativeToRoot(Seq, PelvisBonsName, NextTime).GetLocation();
		const FVector CurrentFootPos = CurrentFootLoc.ProjectOnToNormal(Dir);
		const FVector NextFootPos = NextFootLoc.ProjectOnToNormal(Dir);
		const FVector CurrentPelvisPos = CurrentPelvisLoc.ProjectOnToNormal(Dir);
		const FVector NextPelvisPos = NextPelvisLoc.ProjectOnToNormal(Dir);
		const float CurrentDot = (CurrentFootPos - CurrentPelvisPos) | Dir;
		const float NextDot = (NextFootPos - NextPelvisPos) | Dir;
		if(CurrentDot >= 0 && NextDot < 0)
		{
			const float Time = CurrentTime;
			UAnimationBlueprintLibrary::AddAnimationSyncMarker(Seq, FootBoneName, Time, NotifyTrackName);
		}
	};

	const int32 Steps = Seq->GetDataModel()->GetNumberOfFrames() * 1000;

	for(const auto FootBoneName : {TEXT("foot_r"), TEXT("foot_l")})
	{
		FVector Dir;
		for(int32 i = 0; i < Steps - 1 ; ++i)
		{
			const float CurrentTime = (Seq->GetDataModel()->GetPlayLength() / Steps)  * i;
			const float NextTime = (Seq->GetDataModel()->GetPlayLength() / Steps)  * (i + 1);

			FTransform RootStartPos, RootEndPos;
			UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), CurrentTime, true, RootStartPos);
			UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), NextTime, true, RootEndPos);
			Dir = RootEndPos.GetLocation() - RootStartPos.GetLocation();
			Dir.Normalize();
			
			PlaceSyncMarkersBetweenTimes(CurrentTime, NextTime, FootBoneName, Dir);
  		}
		if(bAnimLoops)
		{
			PlaceSyncMarkersBetweenTimes(Seq->GetDataModel()->GetPlayLength(), 0, FootBoneName, Dir);
		}
	}
	
	Seq->MarkPackageDirty();
#endif
}

void USoloBlueprintFunctionLibrary::GenerateSpeedCurve(UAnimSequence* Seq, bool bAnimLoops, bool bUseFinalSpeed)
{
	const FName SpeedCurveName = TEXT("Speed");
	UAnimationBlueprintLibrary::RemoveCurve(Seq, SpeedCurveName);
	UAnimationBlueprintLibrary::AddCurve(Seq, SpeedCurveName);

	float Value = 0.f;

	if(!bUseFinalSpeed)
	{
		for(int32 i = 0; i < Seq->GetDataModel()->GetNumberOfFrames() - 1; ++i)
		{
			const float CurrentTime = Seq->GetTimeAtFrame(i);
			const float NextTime = Seq->GetTimeAtFrame(i + 1);
			const float dT = NextTime - CurrentTime;
			FTransform CurrentRootPose, NextRootPose;
			UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), CurrentTime, true, CurrentRootPose);
			UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), NextTime, true, NextRootPose);
			const float Distance = (NextRootPose.GetLocation() - CurrentRootPose.GetLocation()).Size();
			Value = static_cast<int32>((Distance / dT) * 10.f) / 10.f;
			UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, SpeedCurveName, NextTime, Value);
		}
	}
	else
	{
		const float CurrentTime = Seq->GetTimeAtFrame(Seq->GetDataModel()->GetNumberOfFrames() - 2);
		const float NextTime = Seq->GetTimeAtFrame(Seq->GetDataModel()->GetNumberOfFrames() - 1);
		const float dT = NextTime - CurrentTime;
		FTransform CurrentRootPose, NextRootPose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), CurrentTime, true, CurrentRootPose);
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), NextTime, true, NextRootPose);
		const float Distance = (NextRootPose.GetLocation() - CurrentRootPose.GetLocation()).Size();
		Value = static_cast<int32>((Distance / dT) * 10.f) / 10.f;
		UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, SpeedCurveName, NextTime, Value);
	}
	if(bAnimLoops || bUseFinalSpeed)
	{
		UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, SpeedCurveName, 0, Value);
	}
	Seq->MarkPackageDirty();
}

void USoloBlueprintFunctionLibrary::GenerateDistanceCurve(UAnimSequence* Seq)
{
	const FName DistanceCurveName = TEXT("DistanceCurve");
	UAnimationBlueprintLibrary::RemoveCurve(Seq, DistanceCurveName);
	UAnimationBlueprintLibrary::AddCurve(Seq, DistanceCurveName);

	TArray<float> DistanceCurveValues;
	DistanceCurveValues.Reserve(Seq->GetDataModel()->GetNumberOfFrames());
	for(int32 i = 0; i <= Seq->GetDataModel()->GetNumberOfFrames(); ++i)
	{
		const float Time = Seq->GetTimeAtFrame(i);
		FTransform RootPose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), Time, true, RootPose);
		float Distance = RootPose.GetLocation().Size();
		Distance = static_cast<int32>(Distance * 10.f) / 10.f;
		DistanceCurveValues.Add(Distance);
	}

	bool bLastWasReversed = false;
	for(int i = 0; i < DistanceCurveValues.Num() - 1; ++i)
	{
		float& CurrentValue = DistanceCurveValues[i];
		if(const float& NextValue = DistanceCurveValues[i+1]; NextValue < CurrentValue)
		{
			CurrentValue *= -1;
			bLastWasReversed = true;
		}
		else if(bLastWasReversed && CurrentValue > KINDA_SMALL_NUMBER)
		{
			const float& PrevValue = DistanceCurveValues[i-1];
			bLastWasReversed = false;
			CurrentValue = (PrevValue + NextValue) / 2.f;
		}
	}
	for(int32 i = 0; i < DistanceCurveValues.Num(); ++i)
	{
		UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, DistanceCurveName, Seq->GetTimeAtFrame(i), DistanceCurveValues[i]);
	}

	Seq->MarkPackageDirty();
}

void USoloBlueprintFunctionLibrary::GenerateRootMotionCurve(UAnimSequence* Seq)
{
	const FName DistanceCurveName = TEXT("DistanceCurve");
	UAnimationBlueprintLibrary::RemoveCurve(Seq, DistanceCurveName);
	UAnimationBlueprintLibrary::AddCurve(Seq, DistanceCurveName);

	TArray<float> DistanceCurveValues;
	DistanceCurveValues.Reserve(Seq->GetDataModel()->GetNumberOfFrames());
	for(int32 i = 0; i <= Seq->GetDataModel()->GetNumberOfFrames(); ++i)
	{
		const float Time = Seq->GetTimeAtFrame(i);
		FTransform RootPose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), Time, true, RootPose);
		float Distance = RootPose.GetLocation().Size();
		Distance = static_cast<int32>(Distance * 10.f) / 10.f;
		DistanceCurveValues.Add(Distance);
	}
	for(int32 i = 0; i < DistanceCurveValues.Num(); ++i)
	{
		UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, DistanceCurveName, Seq->GetTimeAtFrame(i), DistanceCurveValues[i]);
	}
	Seq->MarkPackageDirty();
}

void USoloBlueprintFunctionLibrary::GenerateRotationCurve(UAnimSequence* Seq)
{
	float Offset;
	FString LeftS, RightS;
	if(Seq->GetName().Split(TEXT("_Left_"), &LeftS, &RightS))
	{
		Offset = FCString::Atoi(*RightS);
	}
	else if(Seq->GetName().Split(TEXT("_L_"), &LeftS, &RightS))
	{
		Offset = FCString::Atoi(*RightS);
	}
	else if(Seq->GetName().Split(TEXT("_Right_"), &LeftS, &RightS))
	{
		Offset = FCString::Atoi(*RightS);
	}
	else if(Seq->GetName().Split(TEXT("_R_"), &LeftS, &RightS))
	{
		Offset = FCString::Atoi(*RightS);
	}
	else
	{
		return;
	}
	const FName RotationCurveName = TEXT("RotationCurve");
	UAnimationBlueprintLibrary::RemoveCurve(Seq, RotationCurveName);
	UAnimationBlueprintLibrary::AddCurve(Seq, RotationCurveName);
	for(int32 i = 0; i < Seq->GetDataModel()->GetNumberOfFrames(); ++i)
	{
		const float Time = Seq->GetTimeAtFrame(i);
		FTransform RootPose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), Time, true, RootPose);
		const float Yaw = FMath::Abs(RootPose.GetRotation().Rotator().Yaw);
		UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, RotationCurveName, Time, Yaw - Offset);
		
	}

	Seq->MarkPackageDirty();
}


bool USoloBlueprintFunctionLibrary::GenerateRotating(UAnimSequence* Seq, bool bDryRun)
{
	if(!Seq)
	{
		return false;
	}
	FSmartName RotationCurveSmartName;
	Seq->GetSkeleton()->GetSmartNameByName(USkeleton::AnimCurveMappingName, TEXT("RotationCurve"), RotationCurveSmartName);
	if(!RotationCurveSmartName.IsValid())
	{
		return false;
	}
	
	const FFloatCurve* RotationCurve = static_cast<const FFloatCurve*>(Seq->GetCurveData().GetCurveData(RotationCurveSmartName.UID));
	if(!RotationCurve)
	{
		return false;
	}


	FSmartName RotatingCurveSmartName;
	const FName RotatingCurveName = TEXT("bRotating");
	Seq->GetSkeleton()->AddSmartNameAndModify(USkeleton::AnimCurveMappingName, RotatingCurveName, RotatingCurveSmartName);	
	const FAnimationCurveIdentifier CurveId(RotatingCurveSmartName, ERawCurveTrackTypes::RCT_Float);

	bool bFound = false;
	if(!bDryRun)
	{
		Seq->GetController().RemoveCurve(CurveId);
		Seq->GetController().AddCurve(CurveId);
		Seq->GetController().SetCurveKey(CurveId, FRichCurveKey{0.f, 1.f, 0.f, 0.f, ERichCurveInterpMode::RCIM_Constant});
	}
	
	for(auto It = RotationCurve->FloatCurve.GetKeyIterator(); It; ++It)
	{
		if(FMath::Abs(It->Value) < KINDA_SMALL_NUMBER)
		{
			bFound = true;
			if(!bDryRun)
			{
				Seq->GetController().SetCurveKey(CurveId, FRichCurveKey{It->Time, 1.f, 0.f, 0.f, ERichCurveInterpMode::RCIM_Constant});
			}
			break;
		}
	}
	
	if(!bDryRun) Seq->MarkPackageDirty();
	return bFound;
}


void USoloBlueprintFunctionLibrary::GenerateDisableSpeedWarpingCurve(UAnimSequence* Seq)
{
	const FName SpeedCurveName = TEXT("Speed");
	UAnimationBlueprintLibrary::RemoveCurve(Seq, SpeedCurveName);
	UAnimationBlueprintLibrary::AddCurve(Seq, SpeedCurveName);

	float StartTime = 0.f, EndTime = Seq->GetDataModel()->GetPlayLength();
	const int32 Steps = Seq->GetDataModel()->GetNumberOfFrames() - 1;
	int32 i = 0;
	for(; i < Steps - 1 ; ++i)
	{
		float CurrentTime = (Seq->GetDataModel()->GetPlayLength() / Steps)  * i;
		float NextTime = (Seq->GetDataModel()->GetPlayLength() / Steps)  * (i + 1);
		float DeltaTime = NextTime - CurrentTime;
		FTransform CurrentRootPose, NextRootPose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), CurrentTime, true, CurrentRootPose);
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), NextTime, true, NextRootPose);
		float Distance = (NextRootPose.GetLocation() - CurrentRootPose.GetLocation()).Size();
		if(float Value = static_cast<int32>((Distance / DeltaTime) * 10.f) / 10.f; Value > 5)
		{
			StartTime = NextTime;
			break;
		}
	}
	i+=10;
	FTransform RootStartPos, RootEndPos;
	UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), 0, true, RootStartPos);
	UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), Seq->GetDataModel()->GetPlayLength(), true, RootEndPos);
	FVector RootDir = RootEndPos.GetLocation() - RootStartPos.GetLocation();
	RootDir.Normalize();
	float FrameTime = Seq->GetTimeAtFrame(1) - Seq->GetTimeAtFrame(0);
	FTransform FootRCurrent = GetBonePoseForTimeRelativeToRoot(Seq, TEXT("foot_r"), StartTime);
	FTransform FootRNext= GetBonePoseForTimeRelativeToRoot(Seq, TEXT("foot_r"), StartTime + FrameTime * 10);
	FTransform FootLCurrent = GetBonePoseForTimeRelativeToRoot(Seq, TEXT("foot_l"), StartTime);
	FTransform FootLNext= GetBonePoseForTimeRelativeToRoot(Seq, TEXT("foot_l"), StartTime + FrameTime * 10);
	FVector FootRMove = FootRNext.GetLocation() - FootRCurrent.GetLocation();
	FVector FootLMove = FootLNext.GetLocation() - FootLCurrent.GetLocation();
	
	//determine which foot is actually moving forward
	bool bUseRight = (FootRMove | RootDir) > 0;
	bool bUseLeft = (FootLMove | RootDir) > 0;
	if(bUseRight && bUseLeft)
	{
		FVector FootRMoveProj = FootRMove.ProjectOnToNormal(RootDir);
		FVector FootLMoveProj = FootLMove.ProjectOnToNormal(RootDir);
		bUseRight = FootRMoveProj.SizeSquared() > FootLMoveProj.SizeSquared();
	}
	
	FName BoneName = bUseRight? TEXT("foot_r") : TEXT("foot_l");
	for(; i < Steps - 1; ++i)
	{
		float CurrentTime = (Seq->GetDataModel()->GetPlayLength() / Steps)  * i;
		float NextTime = (Seq->GetDataModel()->GetPlayLength() / Steps)  * (i + 1);
		
		FTransform CurrentPos = GetBonePoseForTimeRelativeToRoot(Seq, BoneName, CurrentTime);
		FTransform NextPos = GetBonePoseForTimeRelativeToRoot(Seq, BoneName, NextTime);
		FVector BoneDir = NextPos.GetLocation() - CurrentPos.GetLocation();
		BoneDir.Normalize();
		if((BoneDir | RootDir) <= 0)
		{
			EndTime = CurrentTime;
			break;
		}
	}
	FName DisableSpeedWarpingName = TEXT("DisableSpeedWarping");
	UAnimationBlueprintLibrary::RemoveCurve(Seq, DisableSpeedWarpingName);
	UAnimationBlueprintLibrary::AddCurve(Seq, DisableSpeedWarpingName);
	UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, DisableSpeedWarpingName, 0.f, 1.f);
	UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, DisableSpeedWarpingName, Seq->GetDataModel()->GetPlayLength(), 0.f);
	UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, DisableSpeedWarpingName, StartTime, 1.f);
	UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, DisableSpeedWarpingName, EndTime, 0.f);

	Seq->MarkPackageDirty();
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

	const FName RootBoneName = TEXT("root");
	const FName PelvisBoneName = TEXT("pelvis");
	
	FVector Direction = FVector::ZeroVector;
	if(OptionalDirection.IsNearlyZero())
	{
		const FBoneAnimationTrack& Track = Sequence->GetDataModel()->GetBoneTrackByName(RootBoneName);
		FVector LastPosKey = FVector(Track.InternalTrackData.PosKeys.Last());
		FVector FirstPosKey = FVector(Track.InternalTrackData.PosKeys[0]);
		Direction = LastPosKey - FirstPosKey;
	}
	else
	{
		Direction = OptionalDirection;
	}
	Direction.Normalize();
	if(Direction.IsNearlyZero())
	{
		return;
	}

	TArray<FTransform> PelvisTransforms;
	for(int32 i = 0; i <= Sequence->GetDataModel()->GetNumberOfFrames(); ++i)
	{
		FTransform FramePose = GetBonePoseForTimeRelativeToRoot(Sequence, PelvisBoneName, Sequence->GetTimeAtFrame(i), true);
		PelvisTransforms.Add(FramePose);
	}

	TArray<FVector> RootPosKeys;
	TArray<FQuat> RootRotKeys;
	TArray<FVector> RootScaleKeys;
	for(int32 i = 0; i <= Sequence->GetDataModel()->GetNumberOfFrames(); ++i)
	{
		auto DistanceCurveValue = AnimCurve->Evaluate(Sequence->GetTimeAtFrame(i));
		FVector NewPos = Direction * DistanceCurveValue;
		RootPosKeys.Add(NewPos);
		RootRotKeys.Add(FQuat::Identity);
		RootScaleKeys.Add(FVector::OneVector);
	}

	TArray<FVector> PelvisPosKeys;
	TArray<FQuat> PelvisRotKeys;
	TArray<FVector> PelvisScaleKeys;
	for(int32 i = 0; i <= Sequence->GetDataModel()->GetNumberOfFrames(); ++i)
	{
		const FQuat& ParentRot = RootRotKeys[i];
		const FVector& ParentPos = RootPosKeys[i];
		const FVector& ParentScale = RootScaleKeys[i];
		const FTransform ParentToWorld {ParentRot, ParentPos, ParentScale};
		auto RelativeTM = PelvisTransforms[i].GetRelativeTransform(ParentToWorld);
		PelvisPosKeys.Add(RelativeTM.GetLocation());
		PelvisScaleKeys.Add(RelativeTM.GetScale3D());
		PelvisRotKeys.Add(RelativeTM.GetRotation());
	}

	Sequence->GetController().SetBoneTrackKeys(RootBoneName, RootPosKeys, RootRotKeys, RootScaleKeys);
	Sequence->GetController().SetBoneTrackKeys(PelvisBoneName, PelvisPosKeys, PelvisRotKeys, PelvisScaleKeys);
	Sequence->GetController().NotifyPopulated();
	Sequence->MarkPackageDirty();
#endif
}

bool USoloBlueprintFunctionLibrary::GenerateIKBonesFollowFK(UAnimSequence* Seq, const bool bDryRun)
{
	TMap<FName, FName> MatchBones {
			{TEXT("ik_foot_root"), TEXT("root")},
			{TEXT("ik_hand_root"), TEXT("root")},
			{TEXT("ik_foot_l"), TEXT("foot_l")},
			{TEXT("ik_foot_r"), TEXT("foot_r")},
			{TEXT("ik_hand_gun"), TEXT("hand_r")},
			{TEXT("ik_hand_l"), TEXT("hand_l")},
			{TEXT("ik_hand_r"), TEXT("hand_r")}
	};
	
	TMap<FName, FName> BoneParents {
				{TEXT("ik_foot_root"), TEXT("root")},
				{TEXT("ik_hand_root"), TEXT("root")},
				{TEXT("ik_foot_l"), TEXT("ik_foot_root")},
				{TEXT("ik_foot_r"), TEXT("ik_foot_root")},
				{TEXT("ik_hand_gun"), TEXT("ik_hand_root")},
				{TEXT("ik_hand_l"), TEXT("ik_hand_gun")},
				{TEXT("ik_hand_r"), TEXT("ik_hand_gun")}
	};
	
	for(auto& Match : MatchBones)
	{
		TArray<FVector> PosKeys;
		TArray<FQuat> RotKeys;
		TArray<FVector> ScaleKeys;

		for(int i = 0; i <= Seq->GetDataModel()->GetNumberOfFrames(); ++i)
		{
			auto IkTr = GetBonePoseForTimeRelativeToRoot(Seq, Match.Key, Seq->GetTimeAtFrame(i), true);
			auto FKTr = GetBonePoseForTimeRelativeToRoot(Seq, Match.Value, Seq->GetTimeAtFrame(i), true);
			if(!IkTr.Equals(FKTr, 1.f) && bDryRun)
			{
				UE_LOG(LogTemp, Log, TEXT("Unmatched bones %s %s in anim %s"), *Match.Key.ToString(), *Match.Value.ToString(), *Seq->GetName());
				return true;
			}
			auto ParentTr = GetBonePoseForTimeRelativeToRoot(Seq, BoneParents[Match.Key], Seq->GetTimeAtFrame(i), true);
			FTransform RelativeTM = FKTr.GetRelativeTransform(ParentTr);
			PosKeys.Add(RelativeTM.GetLocation());
			RotKeys.Add(RelativeTM.GetRotation());
			ScaleKeys.Add(FVector::OneVector);
		}
		if(!bDryRun)
		{
			Seq->GetController().SetBoneTrackKeys(Match.Key, PosKeys, RotKeys, ScaleKeys);
			Seq->GetController().NotifyPopulated();
			Seq->MarkPackageDirty();
		}
	}
	return false;
}

void USoloBlueprintFunctionLibrary::BuildJumpAnims(UAnimSequence* FullJump, int32 StartBeginFrame, int32 ApexBeginFrame, int32 ApexEndFrame, int32 FallEndFrame)
{
#if WITH_EDITOR
	GenerateRootMotionCurve(FullJump);

	const FName DistanceCurveName = TEXT("DistanceCurve");

	//this calculates a simple fall with constant acceleration from beginning of start section to max z
	FSmartName CurveSmartName;
	FullJump->GetSkeleton()->GetSmartNameByName(USkeleton::AnimCurveMappingName, DistanceCurveName, CurveSmartName);
	const FFloatCurve* AnimCurve = static_cast<const FFloatCurve*>(FullJump->GetCurveData().GetCurveData(CurveSmartName.UID));
	TArray<float> Keys, Values;
	AnimCurve->GetKeys(Keys, Values);
	float* Max = Algo::MaxElement(Values);
	int32 TopFrame = Max - &Values[0];
	float TopFrameTime = Keys[TopFrame];
	TopFrame = FullJump->GetFrameAtTime(TopFrameTime);
	float StartFrameTime = FullJump->GetTimeAtFrame(StartBeginFrame);
	
	float AccelStart = (2 * (*Max)) / ((TopFrameTime - StartFrameTime) * (TopFrameTime - StartFrameTime));

	const FName FakeDistanceCurveName = TEXT("FakeDistanceCurveName");

	UAnimationBlueprintLibrary::RemoveCurve(FullJump, FakeDistanceCurveName);
	UAnimationBlueprintLibrary::AddCurve(FullJump, FakeDistanceCurveName);

	for(int32 i = TopFrame; i >= StartBeginFrame; --i)
	{
		float FrameTime = FullJump->GetTimeAtFrame(TopFrame - i);
		UAnimationBlueprintLibrary::AddFloatCurveKey(FullJump, FakeDistanceCurveName, FullJump->GetTimeAtFrame(i), *Max - (AccelStart * FrameTime * FrameTime) / 2);
	}
	UAnimationBlueprintLibrary::AddFloatCurveKey(FullJump, FakeDistanceCurveName, FullJump->GetTimeAtFrame(StartBeginFrame), 0);
	UAnimationBlueprintLibrary::AddFloatCurveKey(FullJump, FakeDistanceCurveName, 0, 0);
	UAnimationBlueprintLibrary::AddFloatCurveKey(FullJump, FakeDistanceCurveName, TopFrameTime, *Max);
	float FallEndFramTime = FullJump->GetTimeAtFrame(FallEndFrame);

	float AccelFall = (2 * (*Max)) / ((FallEndFramTime - TopFrameTime) * (FallEndFramTime - TopFrameTime));

	for(int32 i = TopFrame; i <= FallEndFrame; ++i)
	{
		float FrameTime = FullJump->GetTimeAtFrame((TopFrame - i) * -1);
		UAnimationBlueprintLibrary::AddFloatCurveKey(FullJump, FakeDistanceCurveName, FullJump->GetTimeAtFrame(i), *Max - (AccelFall * FrameTime * FrameTime) / 2);
	}
	UAnimationBlueprintLibrary::AddFloatCurveKey(FullJump, FakeDistanceCurveName, FullJump->GetTimeAtFrame(FallEndFrame), 0);
	UAnimationBlueprintLibrary::AddFloatCurveKey(FullJump, FakeDistanceCurveName, FullJump->GetPlayLength(), 0);
	UAnimationBlueprintLibrary::AddFloatCurveKey(FullJump, FakeDistanceCurveName, TopFrameTime, *Max);

		
	FString Path = FullJump->GetPathName();
	Path.Split(TEXT("."), &Path, nullptr);
	Path.Split(TEXT("/"), &Path, nullptr, ESearchCase::IgnoreCase, ESearchDir::FromEnd);
	Path += "/";
	
	FString AnimPath = FullJump->GetPathName();
	FString AnimName;
	AnimPath.Split(TEXT("."), &AnimPath, &AnimName);
	UEditorAssetLibrary::DeleteAsset(Path + TEXT("Jump_Start"));
	UEditorAssetLibrary::DeleteAsset(Path + TEXT("Jump_Apex"));
	UEditorAssetLibrary::DeleteAsset(Path + TEXT("Jump_Land"));
	UEditorAssetLibrary::DeleteAsset(Path + TEXT("Jump_Recovery"));
	UEditorAssetLibrary::DeleteAsset(Path + TEXT("Jump_Loop"));
	auto* Jump_Start = Cast<UAnimSequence>(UEditorAssetLibrary::DuplicateAsset(AnimPath, Path + TEXT("Jump_Start")));
	auto* Jump_Apex = Cast<UAnimSequence>(UEditorAssetLibrary::DuplicateAsset(AnimPath, Path + TEXT("Jump_Apex")));
	auto* Jump_Land = Cast<UAnimSequence>(UEditorAssetLibrary::DuplicateAsset(AnimPath, Path + TEXT("Jump_Land")));
	auto* Jump_Recovery = Cast<UAnimSequence>(UEditorAssetLibrary::DuplicateAsset(AnimPath, Path + TEXT("Jump_Recovery")));
	auto* Jump_Loop = Cast<UAnimSequence>(UEditorAssetLibrary::DuplicateAsset(AnimPath, Path + TEXT("Jump_Loop")));
	
	UE::Anim::AnimationData::RemoveKeys(Jump_Start, 0, StartBeginFrame);
	UE::Anim::AnimationData::RemoveKeys(Jump_Start, ApexBeginFrame - StartBeginFrame + 1, Jump_Start->GetDataModel()->GetNumberOfKeys() - (ApexBeginFrame - StartBeginFrame + 1));
	UE::Anim::AnimationData::RemoveKeys(Jump_Apex, 0, ApexBeginFrame);
	UE::Anim::AnimationData::RemoveKeys(Jump_Apex, (ApexEndFrame - ApexBeginFrame + 1), Jump_Apex->GetDataModel()->GetNumberOfKeys() - (ApexEndFrame - ApexBeginFrame + 1));
	UE::Anim::AnimationData::RemoveKeys(Jump_Land, 0, ApexEndFrame);
	UE::Anim::AnimationData::RemoveKeys(Jump_Land, (FallEndFrame - ApexEndFrame + 1), Jump_Land->GetDataModel()->GetNumberOfKeys() - (FallEndFrame - ApexEndFrame + 1));
	UE::Anim::AnimationData::RemoveKeys(Jump_Recovery, 0, FallEndFrame);
	UE::Anim::AnimationData::RemoveKeys(Jump_Loop, 0, ApexEndFrame);
	UE::Anim::AnimationData::RemoveKeys(Jump_Loop, 1, Jump_Loop->GetDataModel()->GetNumberOfKeys() - 1);

	TArray PositionalKeys {{FVector3f::ZeroVector}};
	TArray RotationalKeys {{FQuat4f::Identity}};
	TArray ScalingKeys {{FVector3f::OneVector}};
	Jump_Start->GetController().SetBoneTrackKeys(TEXT("root"), PositionalKeys, RotationalKeys, ScalingKeys);
	Jump_Start->GetController().NotifyPopulated();
	Jump_Apex->GetController().SetBoneTrackKeys(TEXT("root"), PositionalKeys, RotationalKeys, ScalingKeys);
	Jump_Apex->GetController().NotifyPopulated();
	Jump_Land->GetController().SetBoneTrackKeys(TEXT("root"), PositionalKeys, RotationalKeys, ScalingKeys);
	Jump_Land->GetController().NotifyPopulated();
	Jump_Recovery->GetController().SetBoneTrackKeys(TEXT("root"), PositionalKeys, RotationalKeys, ScalingKeys);
	Jump_Recovery->GetController().NotifyPopulated();
	Jump_Loop->GetController().SetBoneTrackKeys(TEXT("root"), PositionalKeys, RotationalKeys, ScalingKeys);
	Jump_Loop->GetController().NotifyPopulated();
	
#endif
}

FTransform USoloBlueprintFunctionLibrary::GetBonePoseForTimeRelativeToRoot(const UAnimSequence* Seq, const FName Bone, const float Time, const bool bUseRoot)
{
	TArray<FName> Path;
	UAnimationBlueprintLibrary::FindBonePathToRoot(Seq, Bone, Path);
	FTransform Result = FTransform::Identity;
	for(const auto& B : Path)
	{
		FTransform FramePose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, B, Time, bUseRoot, FramePose);
		Result = Result * FramePose;
	}
	return Result;
}
