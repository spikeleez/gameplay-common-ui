// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CommonUISettings.h"
#include "ICommonUIModule.h"

namespace GameplayCommonSettingsHelpers
{
	inline bool HasPlatformTrait(const FGameplayTag& Tag)
	{
		return ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(Tag);
	}

	inline bool HasPlatformTraitExact(const FGameplayTag& Tag)
	{
		return ICommonUIModule::GetSettings().GetPlatformTraits().HasTagExact(Tag);
	}
}
