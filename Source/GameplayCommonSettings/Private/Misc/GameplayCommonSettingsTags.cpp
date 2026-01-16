// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Misc/GameplayCommonSettingsTags.h"

namespace GameplayCommonSettingsTags
{
	UE_DEFINE_GAMEPLAY_TAG(Trait_SupportsWindowedMode, "Platform.Trait.SupportsWindowedMode");
	UE_DEFINE_GAMEPLAY_TAG(Trait_SupportsChangingAudioOutputDevice, "Platform.Trait.SupportsChangingAudioOutputDevice");
	UE_DEFINE_GAMEPLAY_TAG(Trait_BinauralSettingControlledByOS, "Platform.Trait.BinauralSettingControlledByOS");
	UE_DEFINE_GAMEPLAY_TAG(Trait_SupportsBackgroundAudio, "Platform.Trait.SupportsBackgroundAudio");
	UE_DEFINE_GAMEPLAY_TAG(Trait_NeedsBrightnessAdjustment, "Platform.Trait.NeedsBrightnessAdjustment");
	UE_DEFINE_GAMEPLAY_TAG(Trait_SupportsLatencyStats, "Platform.Trait.SupportsLatencyStats");
	UE_DEFINE_GAMEPLAY_TAG(Trait_SupportsLatencyMarkers, "Platform.Trait.SupportsLatencyMarkers");
	
	UE_DEFINE_GAMEPLAY_TAG(Action_EditSafeZone, "UI.Action.EditSafeZone");
	UE_DEFINE_GAMEPLAY_TAG(Action_EditBrightness, "UI.Action.EditBrightness");
}