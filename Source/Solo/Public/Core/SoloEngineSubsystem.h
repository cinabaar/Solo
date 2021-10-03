// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "SoloEngineSubsystem.generated.h"

/**
 * 
 */
UCLASS()
class SOLO_API USoloEngineSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
