// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplaySettingEditCondition.h"
#include "Engine/LocalPlayer.h"

/**
 * @brief Edit condition that restricts settings to the primary local player
 * 
 * This condition disables settings if the local player is not the primary player
 * (e.g., in split-screen scenarios). This is typically used for global settings
 * like graphics quality that affect all players but should only be controlled
 * by the primary user.
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingWhenPlayingAsPrimaryPlayer : public FGameplaySettingEditCondition
{
public:
	/**
	 * @brief Gets a shared reference to the primary player edit condition
	 * @return A shared reference to the condition
	 */
	static TSharedRef<FGameplaySettingWhenPlayingAsPrimaryPlayer> Get();

	// ~Begin FGameplaySettingEditCondition interface
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState) const override;
	// ~End of FGameplaySettingEditCondition interface
};