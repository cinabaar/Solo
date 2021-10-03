// Fill out your copyright notice in the Description page of Project Settings.


#include "Camera/SoloPlayerCameraManager.h"
#include "Camera/SoloCameraActor.h"

void ASoloPlayerCameraManager::InitializeFor(APlayerController* PC)
{
	Super::InitializeFor(PC);
	// if(CameraActorClass && !PC->bAutoManageActiveCameraTarget)
	// {
	// 	CameraActor = GetWorld()->SpawnActorDeferred<ASoloCameraActor>(CameraActorClass, FTransform::Identity, this, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	// 	CameraActor->InitializeFor(PC);
	// 	CameraActor->FinishSpawning(FTransform::Identity, true);
	// 	
	// 	FViewTargetTransitionParams ViewTargetTransitionParams;
	// 	ViewTargetTransitionParams.BlendTime = 0.f;
	// 	SetViewTarget(CameraActor, ViewTargetTransitionParams);
	// 	UpdateCamera(0.f);
	// }
}
