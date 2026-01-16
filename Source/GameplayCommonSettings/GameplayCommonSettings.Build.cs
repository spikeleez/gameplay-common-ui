// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class GameplayCommonSettings : ModuleRules
{
	public GameplayCommonSettings(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			"InputCore",
			"GameplayTags",
			"CommonUI",
			"CommonInput",
			"DeveloperSettings",
			"UMG",
			"EnhancedInput",
			"RHI",
			
			"AudioModulation",
			"AudioMixer",
			
			"GameplayCommonUI",
		});
			
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"ApplicationCore",
			"PropertyPath"
		});
	}
}
