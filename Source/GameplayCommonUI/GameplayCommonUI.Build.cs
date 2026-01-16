// Copyright Spike Plugins 2026. All Rights Reserved.

using UnrealBuildTool;

public class GameplayCommonUI : ModuleRules
{
	public GameplayCommonUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"CommonUI",
			"CommonInput",
			"GameplayTags",
			"UMG",
			"EnhancedInput",
			"Overlay",
			"MediaAssets"
		});
		
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"DeveloperSettings",
			"ApplicationCore"
		});
	}
}
