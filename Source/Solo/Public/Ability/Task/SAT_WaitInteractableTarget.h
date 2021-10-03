// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/Tasks/AbilityTask.h"
#include "SAT_WaitInteractableTarget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitInteractableTargetDelegate, const FGameplayAbilityTargetDataHandle&, Data);

UCLASS()
class SOLO_API USAT_WaitInteractableTarget : public UAbilityTask
{
	GENERATED_BODY()


	UPROPERTY(BlueprintAssignable)
	FWaitInteractableTargetDelegate FoundNewInteractableTarget;

	UPROPERTY(BlueprintAssignable)
	FWaitInteractableTargetDelegate LostInteractableTarget;
	/**
	* Finds interactable targets around the actor.
	* @param MaxRange How far to search.
	* @param TimerPeriod Period of search timer.
	* @param bShowDebug Draws debug lines for search.
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static USAT_WaitInteractableTarget* WaitForInteractableTarget(UGameplayAbility* OwningAbility, FName TaskInstanceName, float MaxRange = 200.0f, float TimerPeriod = 0.1f, bool bShowDebug = true);

	virtual void Activate() override;

protected:
	virtual void OnDestroy(bool AbilityEnded) override;
	FGameplayAbilityTargetingLocationInfo StartLocation;
	FGameplayAbilityTargetDataHandle TargetData;
	float MaxRange;
	float TimerPeriod;
	bool bShowDebug;
	FTimerHandle SearchTimerHandle;
	void PerformSearch();
};

UCLASS()
class SOLO_API USAT_WaitInteractableTargetLost : public UAbilityTask
{
	GENERATED_BODY()

	UPROPERTY(BlueprintAssignable)
	FWaitInteractableTargetDelegate LostInteractableTarget;
	/**
	* Finds interactable targets around the actor.
	* @param MaxRange How far to search.
	* @param TimerPeriod Period of search timer.
	* @param bShowDebug Draws debug lines for search.
	*/
	UFUNCTION(BlueprintCallable, meta = (HidePin = "OwningAbility", DefaultToSelf = "OwningAbility", BlueprintInternalUseOnly = "true", HideSpawnParms = "Instigator"), Category = "Ability|Tasks")
	static USAT_WaitInteractableTargetLost* WaitUntilInteractableTargetLost(UGameplayAbility* OwningAbility, FName TaskInstanceName, const FGameplayAbilityTargetDataHandle& TargetData, float MaxRange = 200.0f, float TimerPeriod = 0.1f, bool bShowDebug = true);

	virtual void Activate() override;

protected:
	virtual void OnDestroy(bool AbilityEnded) override;
	FGameplayAbilityTargetingLocationInfo StartLocation;
	FGameplayAbilityTargetDataHandle TargetData;
	float MaxRange;
	float TimerPeriod;
	bool bShowDebug;
	FTimerHandle SearchTimerHandle;
	void Lost();
	void CheckLost();
};
