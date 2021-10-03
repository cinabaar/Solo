// Fill out your copyright notice in the Description page of Project Settings.


#include "LD/SoloLevelScriptActor.h"

#include "Kismet/GameplayStatics.h"
#include "LD/Interactable.h"

ASoloLevelScriptActor::ASoloLevelScriptActor()
{
}

void ASoloLevelScriptActor::BeginPlay()
{
	Super::BeginPlay();
	
	UGameplayStatics::GetAllActorsWithInterface(this, UInteractable::StaticClass(), Interactables);
	for(auto* Interactable : Interactables)
	{
		Interactable->OnEndPlay.AddDynamic(this, &ASoloLevelScriptActor::OnInteractableEndPlay);
	}
	OnActorSpawnedDelegateHandle = GetWorld()->AddOnActorSpawnedHandler(FOnActorSpawned::FDelegate::CreateWeakLambda(this, [&](AActor* InActor)
	{
		if(InActor->Implements<UInteractable>())
		{
			InActor->OnEndPlay.AddDynamic(this, &ASoloLevelScriptActor::OnInteractableEndPlay);
			Interactables.Add(InActor);
		}
	}));
}

void ASoloLevelScriptActor::OnInteractableEndPlay(AActor* Actor , EEndPlayReason::Type EndPlayReason)
{
	Interactables.Remove(Actor);
}

void ASoloLevelScriptActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	GetWorld()->RemoveOnActorSpawnedHandler(OnActorSpawnedDelegateHandle);
}
