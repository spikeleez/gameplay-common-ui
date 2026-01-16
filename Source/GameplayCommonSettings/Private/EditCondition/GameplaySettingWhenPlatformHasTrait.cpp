// Copyright Spike Plugins 2026. All Rights Reserved.

#include "EditCondition/GameplaySettingWhenPlatformHasTrait.h"
#include "EditCondition/GameplaySettingEditableState.h"
#include "CommonUIVisibilitySubsystem.h"
#include "Engine/LocalPlayer.h"

FGameplaySettingWhenPlatformHasTrait::FGameplaySettingWhenPlatformHasTrait()
{
	VisibilityTag = FGameplayTag::EmptyTag;
	bTagDesired = false;
}

TSharedRef<FGameplaySettingWhenPlatformHasTrait> FGameplaySettingWhenPlatformHasTrait::KillIfMissing(FGameplayTag InVisibilityTag, const FString& InKillReason)
{
	check(InVisibilityTag.IsValid());
	check(!InKillReason.IsEmpty());

	TSharedRef<FGameplaySettingWhenPlatformHasTrait> Result = MakeShared<FGameplaySettingWhenPlatformHasTrait>();
	Result->VisibilityTag = InVisibilityTag;
	Result->KillReason = InKillReason;
	Result->bTagDesired = true;

	return Result;
}

TSharedRef<FGameplaySettingWhenPlatformHasTrait> FGameplaySettingWhenPlatformHasTrait::DisableIfMissing(FGameplayTag InVisibilityTag, const FText& InDisableReason)
{
	check(InVisibilityTag.IsValid());
	check(!InDisableReason.IsEmpty());

	TSharedRef<FGameplaySettingWhenPlatformHasTrait> Result = MakeShared<FGameplaySettingWhenPlatformHasTrait>();
	Result->VisibilityTag = InVisibilityTag;
	Result->DisableReason = InDisableReason;
	Result->bTagDesired = true;

	return Result;
}

TSharedRef<FGameplaySettingWhenPlatformHasTrait> FGameplaySettingWhenPlatformHasTrait::KillIfPresent(FGameplayTag InVisibilityTag, const FString& InKillReason)
{
	check(InVisibilityTag.IsValid());
	check(!InKillReason.IsEmpty());

	TSharedRef<FGameplaySettingWhenPlatformHasTrait> Result = MakeShared<FGameplaySettingWhenPlatformHasTrait>();
	Result->VisibilityTag = InVisibilityTag;
	Result->KillReason = InKillReason;
	Result->bTagDesired = false;

	return Result;
}

TSharedRef<FGameplaySettingWhenPlatformHasTrait> FGameplaySettingWhenPlatformHasTrait::DisableIfPresent(FGameplayTag InVisibilityTag, const FText& InDisableReason)
{
	check(InVisibilityTag.IsValid());
	check(!InDisableReason.IsEmpty());

	TSharedRef<FGameplaySettingWhenPlatformHasTrait> Result = MakeShared<FGameplaySettingWhenPlatformHasTrait>();
	Result->VisibilityTag = InVisibilityTag;
	Result->DisableReason = InDisableReason;
	Result->bTagDesired = false;

	return Result;
}

void FGameplaySettingWhenPlatformHasTrait::GatherEditState(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState) const
{
	if (UCommonUIVisibilitySubsystem::GetChecked(InLocalPlayer)->HasVisibilityTag(VisibilityTag) != bTagDesired)
	{
		if (KillReason.IsEmpty())
		{
			InOutEditState.Disable(DisableReason);
		}
		else
		{
			InOutEditState.Kill(KillReason);
		}
	}
}
