// Fill out your copyright notice in the Description page of Project Settings.


#include "Util/SoloBlueprintFunctionLibrary.h"

#include "Animation/AimOffsetBlendSpace.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimNode_StateMachine.h"
#include "Animation/AnimStateMachineTypes.h"
#include "Animation/BlendSpaceBase.h"
#include "Internationalization/Regex.h"
#include "Animation/AnimSequence.h"
#if WITH_EDITOR
#include "ScopedTransaction.h"
#include "AnimationModifiers/Public/AnimationBlueprintLibrary.h"
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
					auto* NewAnim = UEditorAssetLibrary::DuplicateAsset(AnimPath, Path + NewAnimName);
					if(NewAnim)
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

				auto GetSampleName = [](FVector SampleValue) -> FString
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
						auto* NewAnim = UEditorAssetLibrary::DuplicateAsset(AnimPath, Path + NewAnimName);
						if(NewAnim)
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
		float TimeBetweenSyncMarkers = Seq->SequenceLength;
		for(int i = 0; i < Seq->AuthoredSyncMarkers.Num() - 1; ++i)
		{
			float TimeBetween = (Seq->AuthoredSyncMarkers[i+1].Time - Seq->AuthoredSyncMarkers[i].Time);
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
		else if(!bLooping && LastSyncMarkerTime < Seq->SequenceLength * 0.8f)
		{
			AddRow(Seq, FString::Printf(TEXT("Sync marker too far from end in non-looping animation %s. Won't work too well"), *Seq->GetName()), false, [=](){});
		}
		else if(RSync != LSync || bRepeatingSyncMarkers)
		{
			AddRow(Seq, FString::Printf(TEXT("Uneven amount of sync markers in %s. Please fix"), *Seq->GetName()), false, [=](){});
		}
		else if(TimeBetweenSyncMarkers < Seq->SequenceLength * 0.10f)
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
		auto* Skeleton = Seq->GetSkeleton();
		FSmartName CurveSmartName;
		bool bCurveExists = Skeleton->GetSmartNameByName(USkeleton::AnimCurveMappingName, CurveName, CurveSmartName);
		float StartCurveValue = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(Seq, CurveName, 0);
		float EndCurveValue = USoloBlueprintFunctionLibrary::GetAnimCurveValueAtTime(Seq, CurveName, Seq->SequenceLength);
		bool bStartAnim = Type.Contains(TEXT("Start"));
		bool bShouldHaveEvenSpeedAtEnds = CurveName == TEXT("Speed") && (bStartAnim || bLooping);
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
					USoloBlueprintFunctionLibrary::GenerateRotating(Seq);
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
		for(int32 i = 0; i < Seq->GetNumberOfFrames(); ++i)
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
		for(auto& Anim : Anims)
		{
			if(!ValidateExists(Anim.Value, Type, EnumToString(Anim.Key)))
			{
				continue;
			}
			for(auto Curve : Curves)
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
			bool bValidPreview = AO->PreviewBasePose == AnimSet->Idle;
			if(!bValidPreview)
			{
				AddRow(AO, FString::Printf(TEXT("Anim %s invalid preview base pose."), *AO->GetName()), true, [=]()
				{
					AO->PreviewBasePose = AnimSet->Idle;
					AO->MarkPackageDirty();
				});
			}
			bool bValidSamples = AO->GetBlendSamples().Num() > 0;
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
	for(auto& Anim : AnimSet->TurnAnims)
	{
		if(!ValidateExists(Anim.Value, TEXT("Turn"), FString::FromInt(Anim.Key)))
		{
			continue;
		}

		ValidateIKBones(Anim.Value);
		ValidateRootNotMoving(Anim.Value);
		FRegexMatcher Matcher(TurnAnimNamePattern, Anim.Value->GetName());
		if(!Matcher.FindNext())
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
		auto HorizontalBox = SNew(SHorizontalBox);
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
			const FFloatCurve* AnimCurve = static_cast<const FFloatCurve*>(Seq->GetCurveData().GetCurveData(CurveSmartName.UID));
			if(!AnimCurve)
			{
				continue;
			}
			auto& Slot = HorizontalBox->AddSlot();
			Slot.AutoWidth()
			[
				SNew(SButton).Text(FText::FromName(Name)).OnClicked_Lambda([&DetailBuilder, Name, Seq]()
				{
					UAnimationBlueprintLibrary::RemoveCurve(Seq, Name);
					Seq->RefreshCurveData();
					Seq->MarkRawDataAsModified();
					Seq->OnRawDataChanged();
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
		if(FObjectProperty* ObjProp = CastField<FObjectProperty>(*It))
		{
			UObject* Object = ObjProp->GetObjectPropertyValue(ObjProp->ContainerPtrToValuePtr<void>(AnimSet, 0));
			UAnimSequence* Seq = Cast<UAnimSequence>(Object);
			if(Seq)
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

bool USoloBlueprintFunctionLibrary::IsValidDirection(ECardinalDirection Dir)
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
		if(EvalValue >= Value)
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

bool USoloBlueprintFunctionLibrary::FindDirectionChangeTime(UAnimSequence* Seq, float& Time)
{
	TArray<float> DistanceCurveValues;
	DistanceCurveValues.Reserve(Seq->GetNumberOfFrames());
	for(int32 i = 0; i < Seq->GetNumberOfFrames(); ++i)
	{
		FTransform RootPose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), Seq->GetTimeAtFrame(i), true, RootPose);
		float Distance = RootPose.GetLocation().Size();
		Distance = int32(Distance * 10.f) / 10.f;
		DistanceCurveValues.Add(Distance);
	}

	bool bLastWasReversed = false;
	bool bFoundChange = false;
	for(int i = 0; i < DistanceCurveValues.Num() - 1; ++i)
	{
		float& CurrentValue = DistanceCurveValues[i];
		const float& NextValue = DistanceCurveValues[i+1];
		if(NextValue < CurrentValue)
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

	FName PelvisBonsName = TEXT("pelvis");
	FName NotifyTrackName = TEXT("SyncMarkers");

	UAnimationBlueprintLibrary::RemoveAnimationNotifyTrack(Seq, NotifyTrackName);
	UAnimationBlueprintLibrary::AddAnimationNotifyTrack(Seq, NotifyTrackName);

	auto PlaceSyncMarkersBetweenTimes = [&](float CurrentTime, float NextTime, FName FootBoneName, FVector Dir)
	{
		FVector CurrentFootLoc = GetBonePoseForTimeRelativeToRoot(Seq, FootBoneName, CurrentTime).GetLocation();
		FVector NextFootLoc= GetBonePoseForTimeRelativeToRoot(Seq, FootBoneName, NextTime).GetLocation();
		FVector CurrentPelvisLoc = GetBonePoseForTimeRelativeToRoot(Seq, PelvisBonsName, CurrentTime).GetLocation();
		FVector NextPelvisLoc = GetBonePoseForTimeRelativeToRoot(Seq, PelvisBonsName, NextTime).GetLocation();
		FVector CurrentFootPos = CurrentFootLoc.ProjectOnToNormal(Dir);
		FVector NextFootPos = NextFootLoc.ProjectOnToNormal(Dir);
		FVector CurrentPelvisPos = CurrentPelvisLoc.ProjectOnToNormal(Dir);
		FVector NextPelvisPos = NextPelvisLoc.ProjectOnToNormal(Dir);
		float CurrentDot = (CurrentFootPos - CurrentPelvisPos) | Dir;
		float NextDot = (NextFootPos - NextPelvisPos) | Dir;
		if(CurrentDot >= 0 && NextDot < 0)
		{
			float Time = CurrentTime;
			UAnimationBlueprintLibrary::AddAnimationSyncMarker(Seq, FootBoneName, Time, NotifyTrackName);
		}
	};

	const int32 Steps = Seq->GetNumberOfFrames() * 1000;

	for(auto FootBoneName : {TEXT("foot_r"), TEXT("foot_l")})
	{
		FVector Dir;
		for(int32 i = 0; i < Steps - 1 ; ++i)
		{
			float CurrentTime = (Seq->SequenceLength / Steps)  * i;
			float NextTime = (Seq->SequenceLength / Steps)  * (i + 1);

			FTransform RootStartPos, RootEndPos;
			UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), CurrentTime, true, RootStartPos);
			UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), NextTime, true, RootEndPos);
			Dir = RootEndPos.GetLocation() - RootStartPos.GetLocation();
			Dir.Normalize();
			
			PlaceSyncMarkersBetweenTimes(CurrentTime, NextTime, FootBoneName, Dir);
  		}
		if(bAnimLoops)
		{
			PlaceSyncMarkersBetweenTimes(Seq->SequenceLength, 0, FootBoneName, Dir);
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
		for(int32 i = 0; i < Seq->GetNumberOfFrames() - 1; ++i)
		{
			float CurrentTime = Seq->GetTimeAtFrame(i);
			float NextTime = Seq->GetTimeAtFrame(i + 1);
			float dT = NextTime - CurrentTime;
			FTransform CurrentRootPose, NextRootPose;
			UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), CurrentTime, true, CurrentRootPose);
			UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), NextTime, true, NextRootPose);
			float Distance = (NextRootPose.GetLocation() - CurrentRootPose.GetLocation()).Size();
			Value = int32((Distance / dT) * 10.f) / 10.f;
			UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, SpeedCurveName, NextTime, Value);
		}
	}
	else
	{
		float CurrentTime = Seq->GetTimeAtFrame(Seq->GetNumberOfFrames() - 2);
		float NextTime = Seq->GetTimeAtFrame(Seq->GetNumberOfFrames() - 1);
		float dT = NextTime - CurrentTime;
		FTransform CurrentRootPose, NextRootPose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), CurrentTime, true, CurrentRootPose);
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), NextTime, true, NextRootPose);
		float Distance = (NextRootPose.GetLocation() - CurrentRootPose.GetLocation()).Size();
		Value = int32((Distance / dT) * 10.f) / 10.f;
		UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, SpeedCurveName, NextTime, Value);
	}
	if(bAnimLoops || bUseFinalSpeed)
	{
		UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, SpeedCurveName, 0, Value);
	}
	Seq->RefreshCurveData();
	Seq->MarkRawDataAsModified();
	Seq->OnRawDataChanged();
	Seq->MarkPackageDirty();
}

void USoloBlueprintFunctionLibrary::GenerateDistanceCurve(UAnimSequence* Seq)
{
	const FName DistanceCurveName = TEXT("DistanceCurve");
	UAnimationBlueprintLibrary::RemoveCurve(Seq, DistanceCurveName);
	UAnimationBlueprintLibrary::AddCurve(Seq, DistanceCurveName);

	TArray<float> DistanceCurveValues;
	DistanceCurveValues.Reserve(Seq->GetNumberOfFrames());
	for(int32 i = 0; i < Seq->GetNumberOfFrames(); ++i)
	{
		float Time = Seq->GetTimeAtFrame(i);
		FTransform RootPose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), Time, true, RootPose);
		float Distance = RootPose.GetLocation().Size();
		Distance = int32(Distance * 10.f) / 10.f;
		DistanceCurveValues.Add(Distance);
	}

	bool bLastWasReversed = false;
	for(int i = 0; i < DistanceCurveValues.Num() - 1; ++i)
	{
		float& CurrentValue = DistanceCurveValues[i];
		const float& NextValue = DistanceCurveValues[i+1];
		if(NextValue < CurrentValue)
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
	for(int32 i = 0; i < Seq->GetNumberOfFrames(); ++i)
	{
		UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, DistanceCurveName, Seq->GetTimeAtFrame(i), DistanceCurveValues[i]);
	}
	Seq->RefreshCurveData();
	Seq->MarkRawDataAsModified();
	Seq->OnRawDataChanged();
	Seq->MarkPackageDirty();
}

void USoloBlueprintFunctionLibrary::GenerateRotationCurve(UAnimSequence* Seq)
{
	float Offset = 0.f;
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
	for(int32 i = 0; i < Seq->GetNumberOfFrames(); ++i)
	{
		float Time = Seq->GetTimeAtFrame(i);
		FTransform RootPose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), Time, true, RootPose);
		float Yaw = FMath::Abs(RootPose.GetRotation().Rotator().Yaw);
		UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, RotationCurveName, Time, Yaw - Offset);
		
	}
	Seq->RefreshCurveData();
	Seq->MarkRawDataAsModified();
	Seq->OnRawDataChanged();
	Seq->MarkPackageDirty();
}


void USoloBlueprintFunctionLibrary::GenerateRotating(UAnimSequence* Seq)
{
	if(!Seq)
	{
		return;
	}
	FSmartName RotationCurveSmartName;
	Seq->GetSkeleton()->GetSmartNameByName(USkeleton::AnimCurveMappingName, TEXT("RotationCurve"), RotationCurveSmartName);
	if(!RotationCurveSmartName.IsValid())
	{
		return;
	}
	const FFloatCurve* RotationCurve = static_cast<const FFloatCurve*>(Seq->GetCurveData().GetCurveData(RotationCurveSmartName.UID));
	if(!RotationCurve)
	{
		return;
	}
	const FName RotatingCurveName = TEXT("bRotating");
	UAnimationBlueprintLibrary::RemoveCurve(Seq, RotatingCurveName);
	UAnimationBlueprintLibrary::AddCurve(Seq, RotatingCurveName);
	UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, RotatingCurveName, 0, 1.f);

	for(auto It = RotationCurve->FloatCurve.GetKeyIterator(); It; ++It)
	{
		if(FMath::Abs(It->Value) < KINDA_SMALL_NUMBER)
		{
			UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, RotatingCurveName, It->Time, 0.f);
			break;
		}
	}
	FSmartName RotatingCurveSmartName;
	Seq->GetSkeleton()->GetSmartNameByName(USkeleton::AnimCurveMappingName, RotatingCurveName, RotatingCurveSmartName);
	FFloatCurve* RotatingCurve = static_cast<FFloatCurve*>(Seq->RawCurveData.GetCurveData(RotatingCurveSmartName.UID));
	for(auto It = RotatingCurve->FloatCurve.GetKeyHandleIterator(); It; ++It)
	{
		RotatingCurve->FloatCurve.SetKeyInterpMode(*It, ERichCurveInterpMode::RCIM_Constant);
	}
	Seq->RefreshCurveData();
	Seq->MarkRawDataAsModified();
	Seq->OnRawDataChanged();
	Seq->MarkPackageDirty();
}


void USoloBlueprintFunctionLibrary::GenerateDisableSpeedWarpingCurve(UAnimSequence* Seq)
{
	const FName SpeedCurveName = TEXT("Speed");
	UAnimationBlueprintLibrary::RemoveCurve(Seq, SpeedCurveName);
	UAnimationBlueprintLibrary::AddCurve(Seq, SpeedCurveName);

	float StartTime = 0.f, EndTime = Seq->SequenceLength;
	const int32 Steps = Seq->GetNumberOfFrames() - 1;
	int32 i = 0;
	for(; i < Steps - 1 ; ++i)
	{
		float CurrentTime = (Seq->SequenceLength / Steps)  * i;
		float NextTime = (Seq->SequenceLength / Steps)  * (i + 1);
		float dT = NextTime - CurrentTime;
		FTransform CurrentRootPose, NextRootPose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), CurrentTime, true, CurrentRootPose);
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), NextTime, true, NextRootPose);
		float Distance = (NextRootPose.GetLocation() - CurrentRootPose.GetLocation()).Size();
		float Value = int32((Distance / dT) * 10.f) / 10.f;
		if(Value > 5)
		{
			StartTime = NextTime;
			break;
		}
	}
	i+=10;
	FTransform RootStartPos, RootEndPos;
	UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), 0, true, RootStartPos);
	UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, TEXT("root"), Seq->SequenceLength, true, RootEndPos);
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
		float CurrentTime = (Seq->SequenceLength / Steps)  * i;
		float NextTime = (Seq->SequenceLength / Steps)  * (i + 1);
		float dT = NextTime - CurrentTime;
		
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
	UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, DisableSpeedWarpingName, Seq->SequenceLength, 0.f);
	UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, DisableSpeedWarpingName, StartTime, 1.f);
	UAnimationBlueprintLibrary::AddFloatCurveKey(Seq, DisableSpeedWarpingName, EndTime, 0.f);
	Seq->RefreshCurveData();
	Seq->MarkRawDataAsModified();
	Seq->OnRawDataChanged();
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
	const FScopedTransaction Transaction(FText::FromString(TEXT("ApplyDistanceCurveAsRootMotion")));
	//Call modify to restore anim sequence current state
	Sequence->Modify();

	auto GetRawTrackIndex = [Sequence](FName BoneName)
	{
		int32 BoneIndex = Sequence->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName);
		int32 TrackIndex;

		for (TrackIndex = 0; TrackIndex < Sequence->GetRawTrackToSkeletonMapTable().Num(); ++TrackIndex)
		{
			if (Sequence->GetRawTrackToSkeletonMapTable()[TrackIndex].BoneTreeIndex == BoneIndex)
			{
				return TrackIndex;
			}
		}

		return -1;
	};
	FName RootBoneName = TEXT("root");
	FName PelvisBoneName = TEXT("pelvis");

	int32 RootTrackIndex = GetRawTrackIndex(RootBoneName);
	int32 PelvisTrackIndex = GetRawTrackIndex(PelvisBoneName);
	TArray<FTransform> PelvisTransforms;
	
	FVector Direction;
	if(OptionalDirection.IsNearlyZero())
	{
		FRawAnimSequenceTrack& RawTrack = Sequence->GetRawAnimationTrack(RootTrackIndex);
		Direction = RawTrack.PosKeys.Last() - RawTrack.PosKeys[0];
	}
	else
	{
		Direction = OptionalDirection;
	}
	Direction.Normalize();
	FRawAnimSequenceTrack& RawRootTrack = Sequence->GetRawAnimationTrack(RootTrackIndex);
	FRawAnimSequenceTrack& RawPelvisTrack = Sequence->GetRawAnimationTrack(PelvisTrackIndex);

	for(int32 i = 0; i < Sequence->GetNumberOfFrames(); ++i)
	{
		FTransform FramePose = GetBonePoseForTimeRelativeToRoot(Sequence, PelvisBoneName, Sequence->GetTimeAtFrame(i), true);
		PelvisTransforms.Add(FramePose);
	}
	RawRootTrack.PosKeys.Empty();
	for(int32 i = 0; i < Sequence->GetNumberOfFrames(); ++i)
	{
		auto DistanceCurveValue = AnimCurve->Evaluate(Sequence->GetTimeAtFrame(i));
		FVector NewPos = Direction * DistanceCurveValue;
		RawRootTrack.PosKeys.Add(NewPos);
	}
	RawPelvisTrack.PosKeys.Empty();
	RawPelvisTrack.RotKeys.Empty();
	RawPelvisTrack.ScaleKeys.Empty();
	for(int32 i = 0; i < Sequence->GetNumberOfFrames(); ++i)
	{
		const FQuat& ParentRot = RawRootTrack.RotKeys.IsValidIndex(i)? RawRootTrack.RotKeys[i] : RawRootTrack.RotKeys.IsValidIndex(0)? RawRootTrack.RotKeys[0] : FQuat::Identity;
		const FVector& ParentPos = RawRootTrack.PosKeys.IsValidIndex(i)? RawRootTrack.PosKeys[i] : RawRootTrack.PosKeys.IsValidIndex(0)? RawRootTrack.PosKeys[0] : FVector::ZeroVector;
		const FVector& ParentScale = RawRootTrack.ScaleKeys.IsValidIndex(i)? RawRootTrack.ScaleKeys[i] : RawRootTrack.ScaleKeys.IsValidIndex(0)? RawRootTrack.ScaleKeys[0] : FVector::OneVector;
		const FTransform ParentToWorld = {ParentRot, ParentPos, ParentScale};
		FTransform RelativeTM = PelvisTransforms[i].GetRelativeTransform(ParentToWorld);
		RawPelvisTrack.PosKeys.Add(RelativeTM.GetLocation());
		RawPelvisTrack.ScaleKeys.Add(RelativeTM.GetScale3D());
		RawPelvisTrack.RotKeys.Add(RelativeTM.GetRotation());
	}
	Sequence->MarkRawDataAsModified();
	Sequence->OnRawDataChanged();

	Sequence->MarkPackageDirty();
#endif
}

bool USoloBlueprintFunctionLibrary::GenerateIKBonesFollowFK(UAnimSequence* Seq, bool bDryRun)
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
	auto GetRawTrackIndex = [Seq](FName BoneName)
	{
		int32 BoneIndex = Seq->GetSkeleton()->GetReferenceSkeleton().FindBoneIndex(BoneName);
		int32 TrackIndex;

		for (TrackIndex = 0; TrackIndex < Seq->GetRawTrackToSkeletonMapTable().Num(); ++TrackIndex)
		{
			if (Seq->GetRawTrackToSkeletonMapTable()[TrackIndex].BoneTreeIndex == BoneIndex)
			{
				return TrackIndex;
			}
		}

		return -1;
	};
	

	for(auto& Match : MatchBones)
	{
		int32 IKTrackIndex = GetRawTrackIndex(Match.Key);
		FRawAnimSequenceTrack& RawIKTrack = Seq->GetRawAnimationTrack(IKTrackIndex);
		if(!bDryRun)
		{
			RawIKTrack.PosKeys.Empty();
			RawIKTrack.RotKeys.Empty();
		}
		for(int i = 0; i < Seq->GetNumberOfFrames(); ++i)
		{
			auto IkTr = GetBonePoseForTimeRelativeToRoot(Seq, Match.Key, Seq->GetTimeAtFrame(i), true);
			auto FKTr = GetBonePoseForTimeRelativeToRoot(Seq, Match.Value, Seq->GetTimeAtFrame(i), true);
			if(!IkTr.Equals(FKTr, 1.f) && bDryRun)
			{
				UE_LOG(LogTemp, Log, TEXT("Unmatched bones %s %s in anim %s"), *Match.Key.ToString(), *Match.Value.ToString(), *Seq->GetName());
				return true;
			}
			if(!bDryRun)
			{
				auto ParentTr = GetBonePoseForTimeRelativeToRoot(Seq, BoneParents[Match.Key], Seq->GetTimeAtFrame(i), true);
				FTransform RelativeTM = FKTr.GetRelativeTransform(ParentTr);
				RawIKTrack.PosKeys.Add(RelativeTM.GetLocation());
				RawIKTrack.RotKeys.Add(RelativeTM.GetRotation());
			}
		}
		if(!bDryRun)
		{
			Seq->MarkRawDataAsModified();
			Seq->OnRawDataChanged();
			Seq->MarkPackageDirty();
		}
	}
	return false;
}

FTransform USoloBlueprintFunctionLibrary::GetBonePoseForTimeRelativeToRoot(UAnimSequence* Seq, FName Bone, float Time, bool bUseRoot)
{
	TArray<FName> Path;
	UAnimationBlueprintLibrary::FindBonePathToRoot(Seq, Bone, Path);
	FTransform Result = FTransform::Identity;
	for(auto& B : Path)
	{
		FTransform FramePose;
		UAnimationBlueprintLibrary::GetBonePoseForTime(Seq, B, Time, bUseRoot, FramePose);
		Result = Result * FramePose;
	}
	return Result;
}
