// Copyright Spike Plugins 2026. All Rights Reserved.

using UnrealBuildTool;

public class GameplayCommonUIEditor : ModuleRules
{
    public GameplayCommonUIEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "CoreUObject",
            "Engine",
            "Slate",
            "SlateCore"
        });
    }
}