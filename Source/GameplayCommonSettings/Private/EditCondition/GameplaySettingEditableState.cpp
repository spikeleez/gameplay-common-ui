// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "EditCondition/GameplaySettingEditableState.h"

void FGameplaySettingEditableState::Hide(const FString& Reason)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!Reason.IsEmpty(), TEXT("To hide a setting, you must provide a developer reason."));
#endif

	bVisible = false;

#if !UE_BUILD_SHIPPING
	HiddenReasons.Add(Reason);
#endif
}

void FGameplaySettingEditableState::Disable(const FText& Reason)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!Reason.IsEmpty(), TEXT("To disable a setting, you must provide a reason that we can show players."));
#endif

	bEnabled = false;
	DisabledReasons.Add(Reason);
}

void FGameplaySettingEditableState::DisableOption(const FString& Option)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!DisabledOptions.Contains(Option), TEXT("You've already disabled this option."));
#endif
}

void FGameplaySettingEditableState::UnableToReset()
{
	bResetable = false;
}
