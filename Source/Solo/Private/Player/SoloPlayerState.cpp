// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/SoloPlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Ability/SoloAbilitySystemComponent.h"

ASoloPlayerState::ASoloPlayerState()
{
	SessionName = NAME_GameSession;
	NetUpdateFrequency = 100.f;
	// Create ability system component, and set it to be explicitly replicated
	AbilitySystemComponent = CreateDefaultSubobject<USoloAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed mode means we only are replicated the GEs to ourself, not the GEs to simulated proxies. If another GDPlayerState (Hero) receives a GE,
	// we won't be told about it by the Server. Attributes, GameplayTags, and GameplayCues will still replicate to us.
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);
}

void ASoloPlayerState::ClientInitialize(AController* C)
{
	Super::ClientInitialize(C);
}

void ASoloPlayerState::RegisterPlayerWithSession(bool bWasFromInvite)
{
	Super::RegisterPlayerWithSession(bWasFromInvite);
}

void ASoloPlayerState::UnregisterPlayerWithSession()
{
	Super::UnregisterPlayerWithSession();
}

void ASoloPlayerState::SeamlessTravelTo(APlayerState* NewPlayerState)
{
	Super::SeamlessTravelTo(NewPlayerState);
}

UAbilitySystemComponent* ASoloPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ASoloPlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);	
}
