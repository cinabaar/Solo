// Fill out your copyright notice in the Description page of Project Settings.


#include "LD/Interactable.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

// Add default functionality here for any IInteractable functions that are not pure virtual.

bool IInteractable::IsAvailableForInteraction_Implementation() const
{
	return false;
}

float IInteractable::GetInteractionDuration_Implementation() const
{
	return 0.0f;
}

void IInteractable::GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type) const
{
	bShouldSync = false;
	Type = EAbilityTaskNetSyncType::OnlyServerWait;
}

void IInteractable::GetPostInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type) const
{
	bShouldSync = false;
	Type = EAbilityTaskNetSyncType::OnlyServerWait;
}

FSimpleMulticastDelegate* IInteractable::GetTargetCancelInteractionDelegate()
{
	return nullptr;
}

void IInteractable::RegisterInteracter_Implementation(AActor* InteractingActor)
{
	Interacters.AddUnique(InteractingActor);
}

void IInteractable::UnregisterInteracter_Implementation(AActor* InteractingActor)
{
	Interacters.Remove(InteractingActor);
}

void IInteractable::InteractableCancelInteraction_Implementation()
{
	FGameplayTagContainer InteractAbilityTagContainer;
	InteractAbilityTagContainer.AddTag(FGameplayTag::RequestGameplayTag("Ability.Interaction"));

	for (TWeakObjectPtr<AActor>& InteractingActorPtr : Interacters)
	{
		AActor* InteractingActor = InteractingActorPtr.Get();
		if(!InteractingActor)
		{
			continue;
		}
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InteractingActor);

		if (ASC)
		{
			ASC->CancelAbilities(&InteractAbilityTagContainer);
		}
	}

	Interacters.Empty();
}
