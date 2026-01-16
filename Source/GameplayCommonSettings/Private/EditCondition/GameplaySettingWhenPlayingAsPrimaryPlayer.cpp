// Copyright Spike Plugins 2026. All Rights Reserved.

#include "EditCondition/GameplaySettingWhenPlayingAsPrimaryPlayer.h"
#include "EditCondition/GameplaySettingEditableState.h"

#define LOCTEXT_NAMESPACE "GameplayGameSetting"

TSharedRef<FGameplaySettingWhenPlayingAsPrimaryPlayer> FGameplaySettingWhenPlayingAsPrimaryPlayer::Get()
{
	static TSharedRef<FGameplaySettingWhenPlayingAsPrimaryPlayer> Instance = MakeShared<FGameplaySettingWhenPlayingAsPrimaryPlayer>();
	return Instance;
}

void FGameplaySettingWhenPlayingAsPrimaryPlayer::GatherEditState(const ULocalPlayer* InLocalPlayer,FGameplaySettingEditableState& InOutEditState) const
{
	if (!InLocalPlayer->IsPrimaryPlayer())
	{
		InOutEditState.Disable(LOCTEXT("OnlyPrimaryPlayerEditable", "Can only be changed by the primary player."));
	}
}

#undef LOCTEXT_NAMESPACE
