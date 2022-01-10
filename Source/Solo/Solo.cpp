// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "Solo.h"

#include "Modules/ModuleManager.h"
#if WITH_EDITOR
#include "Util/SoloBlueprintFunctionLibrary.h"
#endif

#define LOCTEXT_NAMESPACE "Solo"

DEFINE_LOG_CATEGORY(LogSolo)

class FSoloModuleImpl : public FDefaultGameModuleImpl
{

public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	virtual bool IsGameModule() const override;
};

void FSoloModuleImpl::StartupModule()
{
	UE_LOG(LogSolo, Log, TEXT("StartupGameModule"));
#if WITH_EDITOR
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout( "SoloAnimSetDefinition", FOnGetDetailCustomizationInstance::CreateStatic(&FAnimSetCustomization::MakeInstance));
#endif
}

void FSoloModuleImpl::ShutdownModule()
{
	UE_LOG(LogSolo, Log, TEXT("ShutdownGameModule"));
}

bool FSoloModuleImpl::IsGameModule() const
{
	return true;
}

IMPLEMENT_PRIMARY_GAME_MODULE(FSoloModuleImpl, Solo, "Solo");

#undef LOCTEXT_NAMESPACE