// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MPShooter : ModuleRules
{
	public MPShooter(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate",
			"Niagara"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MPShooter",
			"MPShooter/Shooter",
			"MPShooter/Variant_Platforming",
			"MPShooter/Variant_Platforming/Animation",
			"MPShooter/Variant_Combat",
			"MPShooter/Variant_Combat/AI",
			"MPShooter/Variant_Combat/Animation",
			"MPShooter/Variant_Combat/Gameplay",
			"MPShooter/Variant_Combat/Interfaces",
			"MPShooter/Variant_Combat/UI",
			"MPShooter/Variant_SideScrolling",
			"MPShooter/Variant_SideScrolling/AI",
			"MPShooter/Variant_SideScrolling/Gameplay",
			"MPShooter/Variant_SideScrolling/Interfaces",
			"MPShooter/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
