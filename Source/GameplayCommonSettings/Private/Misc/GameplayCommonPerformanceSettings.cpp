// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Misc/GameplayCommonPerformanceSettings.h"
#include "Engine/PlatformSettingsManager.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayCommonPerformanceSettings)

UGameplayPlatformSpecificRenderingSettings::UGameplayPlatformSpecificRenderingSettings()
{
	MobileFrameRateLimits.Append({ 20, 30, 45, 60, 90, 120 });
}

const UGameplayPlatformSpecificRenderingSettings* UGameplayPlatformSpecificRenderingSettings::Get()
{
	const UGameplayPlatformSpecificRenderingSettings* Result = UPlatformSettingsManager::Get().GetSettingsForPlatform<ThisClass>();
	check(Result);
	return Result;
}

UGameplayCommonPerformanceSettings::UGameplayCommonPerformanceSettings()
{
	PerPlatformSettings.Initialize(UGameplayPlatformSpecificRenderingSettings::StaticClass());

	CategoryName = TEXT("Game");

	DesktopFrameRateLimits.Append({ 30, 60, 120, 144, 160, 165, 180, 200, 240, 360, 500 });

	// Default to all stats are allowed
	FGameplayPerformanceStatGroup& StatGroup = UserFacingPerformanceStats.AddDefaulted_GetRef();
	for (EGameplayDisplayablePerformanceStat PerfStat : TEnumRange<EGameplayDisplayablePerformanceStat>())
	{
		StatGroup.AllowedStats.Add(PerfStat);
	}
}
