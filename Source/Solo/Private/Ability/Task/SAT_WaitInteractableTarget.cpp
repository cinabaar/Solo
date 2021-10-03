// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Task/SAT_WaitInteractableTarget.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Algo/MinElement.h"
#include "Algo/Transform.h"
#include "Character/SoloCharacter.h"
#include "LD/SoloLevelScriptActor.h"

USAT_WaitInteractableTarget* USAT_WaitInteractableTarget::WaitForInteractableTarget(UGameplayAbility* OwningAbility, FName TaskInstanceName, float MaxRange, float TimerPeriod, bool bShowDebug)
{
	USAT_WaitInteractableTarget* Proxy = NewAbilityTask<USAT_WaitInteractableTarget>(OwningAbility, TaskInstanceName);
	Proxy->MaxRange = MaxRange;
	Proxy->TimerPeriod = FMath::Max(0.01f, TimerPeriod);
	Proxy->bShowDebug = bShowDebug;

	ASoloCharacter* Hero = Cast<ASoloCharacter>(OwningAbility->GetCurrentActorInfo()->AvatarActor);

	Proxy->StartLocation = FGameplayAbilityTargetingLocationInfo();
	Proxy->StartLocation.LocationType = EGameplayAbilityTargetingLocationType::SocketTransform;
	Proxy->StartLocation.SourceComponent = Hero->GetMesh();
	Proxy->StartLocation.SourceSocketName = TEXT("");
	Proxy->StartLocation.SourceAbility = OwningAbility;

	return Proxy;
}

void USAT_WaitInteractableTarget::Activate()
{
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(SearchTimerHandle, this, &USAT_WaitInteractableTarget::PerformSearch, TimerPeriod, true);
	PerformSearch();
}

void USAT_WaitInteractableTarget::OnDestroy(bool AbilityEnded)
{
	UWorld* World = GetWorld();
	World->GetTimerManager().ClearTimer(SearchTimerHandle);

	Super::OnDestroy(AbilityEnded);
}

void USAT_WaitInteractableTarget::PerformSearch()
{
	auto* SoloLevel = Cast<ASoloLevelScriptActor>(GetWorld()->GetLevelScriptActor());
	if(!SoloLevel)
	{
		return;
	}
	AActor* SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!SourceActor)
	{
		return;
	}
	TArray<TWeakObjectPtr<AActor>> Interactables;
	const TArray<AActor*>& LevelInteractables = SoloLevel->GetInteractables();
	if(LevelInteractables.Num() > 0)
	{
		AActor* const* ClosestActor = Algo::MinElementBy(LevelInteractables, [&](AActor* InActor)
		{
			return FVector::DistSquared2D(InActor->GetActorLocation(), SourceActor->GetActorLocation());
		});
		float Dist = FVector::DistSquared2D((*ClosestActor)->GetActorLocation(), SourceActor->GetActorLocation());
		if(Dist <= MaxRange * MaxRange)
		{
			Interactables.Add(*ClosestActor);
		}
	}
	bool bBroadcastNewTarget = true;
	if(TargetData.Num() > 0)
	{
		TArray<TWeakObjectPtr<AActor>> OldInteractables = TargetData.Get(0)->GetActors();
		if(OldInteractables.Num() > 0)
		{
			if(OldInteractables == Interactables)
			{
				bBroadcastNewTarget = false;
			}
			else
			{
				LostInteractableTarget.Broadcast(TargetData);
			}
		}
	}
	TargetData = StartLocation.MakeTargetDataHandleFromActors(Interactables, false);
	if(Interactables.Num() > 0 && bBroadcastNewTarget)
	{
		FoundNewInteractableTarget.Broadcast(TargetData);
	}
}

USAT_WaitInteractableTargetLost* USAT_WaitInteractableTargetLost::WaitUntilInteractableTargetLost(UGameplayAbility* OwningAbility, FName TaskInstanceName, const FGameplayAbilityTargetDataHandle& TargetData, float MaxRange, float TimerPeriod, bool bShowDebug)
{
	USAT_WaitInteractableTargetLost* Proxy = NewAbilityTask<USAT_WaitInteractableTargetLost>(OwningAbility, TaskInstanceName);
	Proxy->MaxRange = MaxRange;
	Proxy->TimerPeriod = FMath::Max(0.01f, TimerPeriod);
	Proxy->bShowDebug = bShowDebug;
	Proxy->TargetData = TargetData;
	
	ASoloCharacter* Hero = Cast<ASoloCharacter>(OwningAbility->GetCurrentActorInfo()->AvatarActor);

	Proxy->StartLocation = FGameplayAbilityTargetingLocationInfo();
	Proxy->StartLocation.LocationType = EGameplayAbilityTargetingLocationType::SocketTransform;
	Proxy->StartLocation.SourceComponent = Hero->GetMesh();
	Proxy->StartLocation.SourceSocketName = TEXT("");
	Proxy->StartLocation.SourceAbility = OwningAbility;

	return Proxy;
}

void USAT_WaitInteractableTargetLost::Activate()
{
	UWorld* World = GetWorld();
	World->GetTimerManager().SetTimer(SearchTimerHandle, this, &USAT_WaitInteractableTargetLost::CheckLost, TimerPeriod, true);
	CheckLost();
}

void USAT_WaitInteractableTargetLost::OnDestroy(bool AbilityEnded)
{
	UWorld* World = GetWorld();
	World->GetTimerManager().ClearTimer(SearchTimerHandle);

	Super::OnDestroy(AbilityEnded);
}

void USAT_WaitInteractableTargetLost::Lost()
{
	LostInteractableTarget.Broadcast(TargetData);
	EndTask();
}

void USAT_WaitInteractableTargetLost::CheckLost()
{
	AActor* SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();
	if (!SourceActor)
	{
		Lost();
		return;
	}
	TArray<AActor*> InteractableTargets = UAbilitySystemBlueprintLibrary::GetActorsFromTargetData(TargetData, 0);
	if(InteractableTargets.Num() == 0 || !InteractableTargets[0])
	{
		Lost();
		return;
	}
	auto Dist = FVector::DistSquared2D(InteractableTargets[0]->GetActorLocation(), SourceActor->GetActorLocation());
	if(Dist > MaxRange * MaxRange)
	{
		Lost();
		return;
	}
}
