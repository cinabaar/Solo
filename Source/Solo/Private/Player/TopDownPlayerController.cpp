// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/TopDownPlayerController.h"
#include "VisualLogger/VisualLogger.h"
#include "Engine/LocalPlayer.h"

ATopDownPlayerController::ATopDownPlayerController()
{
	bAutoManageActiveCameraTarget = false;
}

void ATopDownPlayerController::PostProcessInput(const float DeltaTime, const bool bGamePaused)
{
	Super::PostProcessInput(DeltaTime, bGamePaused);
	if (!GetPawn())
	{
		return;
	}

	GetPawn()->AddMovementInput(FVector{ MoveInput, 0.f }, 1.f);
	
// 	if (InputDevice == ESwitcherooInputDevice::Gamepad)
// 	{
// 		if (LookInput != FVector2D::ZeroVector)
// 		{
// 			bAiming = true;
//
// 			FVector PawnLocation = GetPawn()->GetActorLocation();
// 			FVector ProjectedCenterWorldPos, ProjectedCenterWorldDir;
// 			FVector ProjectedInputOffsetPos, ProjectedInputOffsetDir;
//
// 			auto* LP = GetLocalPlayer();
// 			if (LP && LP->ViewportClient)
// 			{
// 				FSceneViewProjectionData ProjectionData;
// 				if (LP->GetProjectionData(LP->ViewportClient->Viewport, /*out*/ ProjectionData))
// 				{
// 					FMatrix const InvViewProjMatrix = ProjectionData.ComputeViewProjectionMatrix().InverseFast();
// 					FVector2D ViewportSize;
// 					LP->ViewportClient->GetViewportSize(ViewportSize);
// 					FSceneView::DeprojectScreenToWorld(ViewportSize / 2.f, ProjectionData.GetConstrainedViewRect(), InvViewProjMatrix, /*out*/ ProjectedCenterWorldPos, /*out*/ ProjectedCenterWorldDir);
// 					FSceneView::DeprojectScreenToWorld(ViewportSize / 2.f + FVector2D{ LookInput.Y, -LookInput.X } *GamepadCursorOffsetMultiplier, ProjectionData.GetConstrainedViewRect(), InvViewProjMatrix, /*out*/ ProjectedInputOffsetPos, /*out*/ ProjectedInputOffsetDir);
// 				}
// 			}
// 			float A = (PawnLocation.Z - ProjectedCenterWorldPos.Z) / ProjectedCenterWorldDir.Z;
// 			float B = (PawnLocation.Z - ProjectedInputOffsetPos.Z) / ProjectedInputOffsetDir.Z;
// 			FVector APos = ProjectedCenterWorldPos + A * ProjectedCenterWorldDir;
// 			FVector BPos = ProjectedInputOffsetPos + B * ProjectedInputOffsetDir;
// 			FVector ScreenInput = (BPos - APos).GetSafeNormal2D() * GamepadCursorOffsetMultiplier;
// #if ENABLE_VISUAL_LOG
// 			FVisualLogger& VLog = FVisualLogger::Get();
// 			if (VLog.IsRecording())
// 			{
// 				UE_VLOG_ARROW(this, LogTemp, Log, PawnLocation, PawnLocation + ControlRotation.Vector() * GamepadCursorOffsetMultiplier, FColor::Blue, TEXT("%s"), *ControlRotation.ToString());
// 				UE_VLOG_ARROW(this, LogTemp, Log, ProjectedCenterWorldPos, ProjectedCenterWorldPos + ProjectedCenterWorldDir * GamepadCursorOffsetMultiplier, FColor::Red, TEXT(""));
// 				UE_VLOG(this, LogTemp, Log, TEXT("%s, %s"), *ProjectedCenterWorldPos.ToString(), *ProjectedCenterWorldDir.ToString());
// 				UE_VLOG_ARROW(this, LogTemp, Log, ProjectedInputOffsetPos, ProjectedInputOffsetPos + ProjectedInputOffsetDir * GamepadCursorOffsetMultiplier, FColor::Green, TEXT(""));
// 				UE_VLOG(this, LogTemp, Log, TEXT("%s, %s"), *ProjectedInputOffsetPos.ToString(), *ProjectedInputOffsetDir.ToString());
// 				UE_VLOG_ARROW(this, LogTemp, Log, PawnLocation, PawnLocation + ScreenInput, FColor::Yellow, TEXT("%s"), *ScreenInput.ToString());
// 				UE_VLOG(this, LogTemp, Log, TEXT("%s"), *ScreenInput.ToString());
// 			}
// #endif
// 			CursorOffset = FVector2D{ ScreenInput };
// 			LookInput = FVector2D::ZeroVector;
// 		}
// 		else
// 		{
// 			CursorOffset = CursorOffset.GetSafeNormal() * GamepadCursorOffsetMultiplier;
// 		}
// 	}
// 	else
// 	{
// 		CursorOffset += FVector2D{MouseInput.Y, MouseInput.X} /** FVector2D{ InputYawScale, InputYawScale }*/;
// 	}

	FVector CursorOffset3D = FVector{ CursorOffset, 0.f };

	UpdateCursorVisualization(GetPawn()->GetActorLocation() + CursorOffset3D);

	if (!CursorOffset.IsNearlyZero() && bAiming)
	{
		FRotator Rot = CursorOffset3D.Rotation();
		//Rot.Pitch = DebugPitch * InputPitchScale;
		SetControlRotation(Rot);
	}
}
