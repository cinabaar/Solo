// Copyright 2020 Dan Kestranek.


#include "Core/SoloEngineSubsystem.h"
#include "AbilitySystemGlobals.h"

void USoloEngineSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UAbilitySystemGlobals::Get().InitGlobalData();
}
