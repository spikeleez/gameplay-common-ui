// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "NativeGameplayTags.h"

/**
 * @brief Native gameplay tags used by the settings system
 * 
 * Includes tags for platform traits (used in edit conditions) and
 * shared action identifiers (used in the UI action interface).
 */
namespace GameplayCommonSettingsTags
{
	/** @brief Trait indicating if the platform supports windowed display mode */
	GAMEPLAYCOMMONSETTINGS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trait_SupportsWindowedMode);
	
	/** @brief Trait indicating if the platform supports change audio output devices */
	GAMEPLAYCOMMONSETTINGS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trait_SupportsChangingAudioOutputDevice);
	
	/** @brief Trait indicating if the platform supports bineural audio */
	GAMEPLAYCOMMONSETTINGS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trait_BinauralSettingControlledByOS);
	
	/** @brief Trait indicating if the platform supports background audio */
	GAMEPLAYCOMMONSETTINGS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trait_SupportsBackgroundAudio);

	/** @brief Trait indicating if the platform needs a manual brightness/gamma adjustment step */
	GAMEPLAYCOMMONSETTINGS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trait_NeedsBrightnessAdjustment);

	/** @brief Trait indicating if the platform supports detailed latency statistics tracking */
	GAMEPLAYCOMMONSETTINGS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trait_SupportsLatencyStats);

	/** @brief Trait indicating if the platform supports hardware latency markers (e.g. NVIDIA Flash) */
	GAMEPLAYCOMMONSETTINGS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Trait_SupportsLatencyMarkers);
	
	/** @brief Action tag for triggering safe zone adjustment UI */
	GAMEPLAYCOMMONSETTINGS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_EditSafeZone);
	
	/** @brief Action tag for triggering brightness adjustment UI */
	GAMEPLAYCOMMONSETTINGS_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_EditBrightness);
}