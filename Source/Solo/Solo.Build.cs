// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class Solo : ModuleRules
{
	public Solo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		bEnforceIWYU = true;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"Switcheroo",
			"AIModule",
			"Strider",
			"AnimGraph"
		});

		PrivateDependencyModuleNames.AddRange(new string[] {
			"GameplayAbilities",
			"GameplayTasks",
			"GameplayTags",
			"OnlineSubsystem",
			"OnlineSubsystemUtils",
			"Slate",
			"SlateCore",
			"UMG",
			"DeveloperSettings",
		});

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] {
				"UnrealEd"
			});
		}
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
