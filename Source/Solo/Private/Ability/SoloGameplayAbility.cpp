// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/SoloGameplayAbility.h"

#include "AbilitySystemComponent.h"

USoloGameplayAbility::USoloGameplayAbility()
{
	// Default to Instance Per Actor
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

    bActivateAbilityOnGranted = false;
    bActivateOnInput = true;

    // UGSAbilitySystemGlobals hasn't initialized tags yet to set ActivationBlockedTags
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.Dead"));
    ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag("State.KnockedDown"));

    ActivationOwnedTags.AddTag(FGameplayTag::RequestGameplayTag("Ability.BlocksInteraction"));

    InteractingTag = FGameplayTag::RequestGameplayTag("State.Interacting");
    InteractingRemovalTag = FGameplayTag::RequestGameplayTag("State.InteractingRemoval");
}

void USoloGameplayAbility::OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec)
{
	if (bActivateAbilityOnGranted)
	{
		bool ActivatedAbility = ActorInfo->AbilitySystemComponent->TryActivateAbility(Spec.Handle, false);
	}
}

bool USoloGameplayAbility::IsInputPressed() const
{
	FGameplayAbilitySpec* Spec = GetCurrentAbilitySpec();
	return Spec && Spec->InputPressed;
}