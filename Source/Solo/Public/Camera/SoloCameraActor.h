// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SoloCameraActor.generated.h"

class UCameraComponent;
class USpringArmComponent;
/**
 * 
 */
UCLASS()
class SOLO_API ASoloCameraActor : public AActor
{
	GENERATED_BODY()
public:
	ASoloCameraActor();
private:
	UPROPERTY(Transient)
	APlayerController* PlayerController = nullptr;
	UPROPERTY(Transient)
	APawn* CachedPawn = nullptr;
protected:
	UPROPERTY(Category = CameraActor, VisibleAnywhere, BlueprintReadOnly)
	UCameraComponent* CameraComponent = nullptr;
	UPROPERTY(Category = CameraActor, VisibleAnywhere, BlueprintReadOnly)
	USceneComponent* SceneComponent = nullptr;
	UPROPERTY(Category = CameraActor, VisibleAnywhere, BlueprintReadOnly)
	USpringArmComponent* SpringArm = nullptr;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	FRotator CameraWorldRotation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	FRotator SpringArmRotation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Camera")
	float TargetArmLength = 1000.f;

	void ControlledPawnChanged(APawn* NewPawn);
	virtual void BeginPlay() override;
public:
	void InitializeFor(APlayerController* PC);
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void Tick(float DeltaSeconds) override;
};
