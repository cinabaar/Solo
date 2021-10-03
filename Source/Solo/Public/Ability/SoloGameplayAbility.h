// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "SoloAbilityInputID.h"
#include "Abilities/GameplayAbility.h"
#include "SoloGameplayAbility.generated.h"

/**
 * 
 */
UCLASS()
class SOLO_API USoloGameplayAbility : public UGameplayAbility
{
	GENERATED_BODY()
public:
	USoloGameplayAbility();
	// Abilities with this set will automatically activate when the input is pressed
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	ESoloAbilityInputID AbilityInputID = ESoloAbilityInputID::None;

	// Value to associate an ability with an slot without tying it to an automatically activated input.
	// Passive abilities won't be tied to an input so we need a way to generically associate abilities with slots.
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Ability")
	ESoloAbilityInputID AbilityID = ESoloAbilityInputID::None;

	// Tells an ability to activate immediately when its granted. Used for passive abilities and abilites forced on others.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bActivateAbilityOnGranted;

	// If true, this ability will activate when its bound input is pressed. Disable if you want to bind an ability to an
	// input but not have it activate when pressed.
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Ability")
	bool bActivateOnInput;

	// If an ability is marked as 'ActivateAbilityOnGranted', activate them immediately when given here
	// Epic's comment: Projects may want to initiate passives or do other "BeginPlay" type of logic here.
	virtual void OnAvatarSet(const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilitySpec& Spec) override;

	UFUNCTION(BlueprintPure, Category = "Ability")
	bool IsInputPressed() const;
protected:
	FGameplayTag InteractingTag;
	FGameplayTag InteractingRemovalTag;
};
