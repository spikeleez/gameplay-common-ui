// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * @brief Represents the editable state of a gameplay setting
 * 
 * This class tracks whether a setting is visible, enabled, and resettable.
 * It also maintains lists of reasons why a setting is disabled or hidden,
 * which can be displayed to the user in the UI.
 * 
 * Edit conditions modify this state during the evaluation process.
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingEditableState
{
public:
	/** @brief Constructs a default editable state (visible, enabled, resettable) */
	FGameplaySettingEditableState()
		: bVisible(true)
		, bEnabled(true)
		, bResetable(true)
		, bHideFromAnalytics(false)
	{
	}

	/** @brief Checks if the setting is visible */
	bool IsVisible() const { return bVisible; }
	
	/** @brief Checks if the setting is enabled for editing */
	bool IsEnabled() const { return bEnabled; }
	
	/** @brief Checks if the setting can be reset to default */
	bool IsResetable() const { return bResetable; }
	
	/** @brief Checks if the setting should be hidden from analytics reporting */
	bool IsHiddenFromAnalytics() const { return bHideFromAnalytics; }
	
	/** @brief Gets the collection of localized reasons why the setting is disabled */
	const TArray<FText>& GetDisabledReasons() const { return DisabledReasons; }

#if !UE_BUILD_SHIPPING
	/** @brief Gets the collection of developer strings explaining why the setting is hidden (non-shipping only) */
	const TArray<FString>& GetHiddenReasons() const { return HiddenReasons; }
#endif

	/** @brief Gets the list of specific options that are disabled (for discrete settings) */
	const TArray<FString>& GetDisabledOptions() const { return DisabledOptions; }

	/** 
	 * @brief Hides the setting
	 * @param Reason Developer message explaining why the setting is hidden
	 */
	void Hide(const FString& Reason);

	/** 
	 * @brief Disables the setting
	 * @param Reason Localized text explaining to the user why the setting is disabled
	 */
	void Disable(const FText& Reason);

	/** 
	 * @brief Disables a specific option within a discrete setting
	 * @param Option The name of the option to disable
	 */
	void DisableOption(const FString& Option);

	/** 
	 * @brief Disables a specific enum option within a discrete setting
	 * @tparam EnumType The enum type
	 * @param InEnumValue The enum value to disable
	 */
	template<typename EnumType>
	void DisableEnumOption(EnumType InEnumValue)
	{
		DisableOption(StaticEnum<EnumType>()->GetNameStringByValue((int64)InEnumValue));
	}

	/** @brief Marks the setting as not resettable through the "Reset to Defaults" action */
	void UnableToReset();

	/**
	 * @brief Hides the setting from analytics reporting
	 * 
	 * Useful for preventing noise from platform-specific settings on unsupported platforms.
	 */
	void HideFromAnalytics() { bHideFromAnalytics = true; }

	/** 
	 * @brief Completely suppresses the setting
	 * 
	 * Hides it visually, marks it as non-resettable, and hides it from analytics.
	 * 
	 * @param Reason Developer message explaining why the setting is killed
	 */
	void Kill(const FString& Reason)
	{
		Hide(Reason);
		HideFromAnalytics();
		UnableToReset();
	}

private:
	/** Visibility state */
	uint8 bVisible:1;

	/** Enabled state */
	uint8 bEnabled:1;

	/** Resettable state */
	uint8 bResetable:1;

	/** Analytics visibility state */
	uint8 bHideFromAnalytics:1;

	/** List of disabled options for discrete settings */
	TArray<FString> DisabledOptions;
	
	/** List of localized reasons for being disabled */
	TArray<FText> DisabledReasons;

#if !UE_BUILD_SHIPPING
	/** List of developer reasons for being hidden */
	TArray<FString> HiddenReasons;
#endif
};