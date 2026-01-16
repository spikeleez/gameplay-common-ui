// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplaySettingEditCondition.h"
#include "GameplayTagContainer.h"

/**
 * @brief Edit condition that evaluates based on platform-specific gameplay tags (traits)
 * 
 * This class checks if the current platform has or lacks a specific gameplay tag.
 * Based on this check, it can either "Kill" (hide/immutable/no-analytics) or 
 * "Disable" the setting.
 * 
 * Commonly used for feature toggles that depend on platform capabilities.
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingWhenPlatformHasTrait : public FGameplaySettingEditCondition
{
public:
	FGameplaySettingWhenPlatformHasTrait();
	
	/**
	 * @brief Hides and kills the setting if the platform lacks the specified tag
	 * @param InVisibilityTag The tag to check for
	 * @param InKillReason Developer reason for killing the setting
	 * @return A shared reference to the new edit condition
	 */
	static TSharedRef<FGameplaySettingWhenPlatformHasTrait> KillIfMissing(FGameplayTag InVisibilityTag, const FString& InKillReason);
	
	/**
	 * @brief Disables the setting if the platform lacks the specified tag
	 * @param InVisibilityTag The tag to check for
	 * @param InDisableReason Localized reason for disabling the setting
	 * @return A shared reference to the new edit condition
	 */
	static TSharedRef<FGameplaySettingWhenPlatformHasTrait> DisableIfMissing(FGameplayTag InVisibilityTag, const FText& InDisableReason);

	/**
	 * @brief Hides and kills the setting if the platform has the specified tag
	 * @param InVisibilityTag The tag to check for
	 * @param InKillReason Developer reason for killing the setting
	 * @return A shared reference to the new edit condition
	 */
	static TSharedRef<FGameplaySettingWhenPlatformHasTrait> KillIfPresent(FGameplayTag InVisibilityTag, const FString& InKillReason);
	
	/**
	 * @brief Disables the setting if the platform has the specified tag
	 * @param InVisibilityTag The tag to check for
	 * @param InDisableReason Localized reason for disabling the setting
	 * @return A shared reference to the new edit condition
	 */
	static TSharedRef<FGameplaySettingWhenPlatformHasTrait> DisableIfPresent(FGameplayTag InVisibilityTag, const FText& InDisableReason);

	// ~Begin FGameplaySettingEditCondition interface
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState) const override;
	// ~End of FGameplaySettingEditCondition interface

private:
	/** The gameplay tag (trait) to check for on the platform */
	FGameplayTag VisibilityTag;

	/** Whether the presence (true) or absence (false) of the tag is the desired state */
	bool bTagDesired;

	/** Developer reason used if the setting is killed */
	FString KillReason;
	
	/** Localized user-facing reason used if the setting is disabled */
	FText DisableReason;
};