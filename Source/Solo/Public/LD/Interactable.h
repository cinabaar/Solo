// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Abilities/Tasks/AbilityTask_NetworkSyncPoint.h"
#include "UObject/Interface.h"
#include "Interactable.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInteractable : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class SOLO_API IInteractable
{
	GENERATED_BODY()

public:
		// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	/**
	* Is this object available for player interaction at right now?
	*
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	bool IsAvailableForInteraction() const;
	virtual bool IsAvailableForInteraction_Implementation() const;

	/**
	* Tell the object to display the possibility of interaction
	*
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void ShowInteractionPrompt(AActor* InteractingActor) const;
	virtual void ShowInteractionPrompt_Implementation(AActor* InteractingActor) const {}

	/**
	* Tell the object to hide the possibility of interaction
	*
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void HideInteractionPrompt(AActor* InteractingActor) const;
	virtual void HideInteractionPrompt_Implementation(AActor* InteractingActor) const {}

	/**
	* How long does the player need to hold down the interact button to interact with this?
	*
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	float GetInteractionDuration() const;
	virtual float GetInteractionDuration_Implementation() const;

	/**
	* Should we sync and who should sync before calling PreInteract()? Defaults to false and OnlyServerWait.
	* OnlyServerWait - client predictively calls PreInteract().
	* OnlyClientWait - client waits for server to call PreInteract(). This is useful if we are activating an ability
	* on another ASC (player) and want to sync actions or animations with our Interact Duration timer.
	* BothWait - client and server wait for each other before calling PreInteract().
	*
	* Player revive uses OnlyClientWait so that the player reviving is in sync with the server since we can't locally
	* predict an ability run on another player. The downed player's reviving animation will be in sync with the local
	* player's Interact Duration Timer.
	*
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void GetPreInteractSyncType(bool& bShouldSync, EAbilityTaskNetSyncType& Type) const;
	virtual void GetPreInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type) const;

	/**
	* Should we sync and who should sync before calling PostInteract()? Defaults to false and OnlyServerWait.
	* OnlyServerWait - client predictively calls PostInteract().
	* OnlyClientWait - client waits for server to call PostInteract().
	* BothWait - client and server wait for each other before calling PostInteract().
	*
	* Player revive uses OnlyServerWait so that the client isn't stuck waiting for the server after the Interaction Duration
	* ends. Revive's PostInteract() will only run code on the server so it's fine for the client to be "finished" ahead of
	* the server.
	*
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void GetPostInteractSyncType(bool& bShouldSync, EAbilityTaskNetSyncType& Type) const;
	virtual void GetPostInteractSyncType_Implementation(bool& bShouldSync, EAbilityTaskNetSyncType& Type) const;

	/**
	* Interact with this Actor. This will call before starting the Interact Duration timer. This might do things, apply
	* (predictively or not) GameplayEffects, trigger (predictively or not) GameplayAbilities, etc.
	*
	* You can use this function to grant abilities that will be predictively activated on PostInteract() to hide the
	* AbilitySpec replication time.
	*
	* If you want to do something predictively, you can get the ASC from the InteractingActor and use its
	* ScopedPredictionKey.
	*
	* Player revives use PreInteract() to trigger a ability that plays an animation that lasts the same duration as
	* the Interact Duration. If this ability finishes, it will revive the player in PostInteract().
	*
	* @param InteractingActor The Actor interacting with this Actor. It will be the AvatarActor from a GameplayAbility.
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void PreInteract(AActor* InteractingActor);
	virtual void PreInteract_Implementation(AActor* InteractingActor) {}

	/**
	* Interact with this Actor. This will call after the Interact Duration timer completes. This might do things, apply
	* (predictively or not) GameplayEffects, trigger (predictively or not) GameplayAbilities, etc.
	*
	* If you want to do something predictively, you can get the ASC from the InteractingActor and use its
	* ScopedPredictionKey.
	*
	* If you need to trigger a GameplayAbility predictively, the player's ASC needs to have been granted the ability
	* ahead of time. If you don't want to grant every possible predictive ability at game start, you can hide the time
	* needed to replicate the AbilitySpec inside the time needed to interact by granted it in PreInteract().
	*
	* @param InteractingActor The Actor interacting with this Actor. It will be the AvatarActor from a GameplayAbility.
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void PostInteract(AActor* InteractingActor);
	virtual void PostInteract_Implementation(AActor* InteractingActor) {}

	/**
	* Cancel an ongoing interaction, typically anything happening in PreInteract() while waiting on the Interact Duration
	* Timer. This will be called if the player releases input early.
	*
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable")
	void CancelInteraction();
	virtual void CancelInteraction_Implementation() {}

	/**
	* Returns a delegate for GA_Interact to bind to that fires when this Actor is canceling the interactiong (e.g. died).
	*
	* @param InteractionComponent UPrimitiveComponent in case an Actor has many separate interactable areas.
	*/
	virtual FSimpleMulticastDelegate* GetTargetCancelInteractionDelegate();

	/**
	* Registers an Actor interacting with this Interactable. Used to send a GameplayEvent to them when this Interactable
	* wishes to cancel interaction prematurely (e.g. a reviving player dies mid-revival). Not meant to be overriden.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable|Do Not Override")
	void RegisterInteracter(AActor* InteractingActor);
	virtual void RegisterInteracter_Implementation(AActor* InteractingActor);

	/**
	* Unregisters an interacting Actor from this Interactable. Not meant to be overriden.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable|Do Not Override")
	void UnregisterInteracter(AActor* InteractingActor);
	virtual void UnregisterInteracter_Implementation(AActor* InteractingActor);

	/**
	* Interactable (or an external Actor, not the Interacter) wants to cancel the interaction (e.g. the reviving player
	* dies mid-revival). Not meant to be overriden.
	*/
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Interactable|Do Not Override")
	void InteractableCancelInteraction();
	virtual void InteractableCancelInteraction_Implementation();

protected:
	TArray<TWeakObjectPtr<AActor>> Interacters;
	
};
