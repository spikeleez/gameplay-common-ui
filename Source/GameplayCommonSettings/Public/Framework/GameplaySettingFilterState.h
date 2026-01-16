// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Misc/TextFilterExpressionEvaluator.h"
#include "GameplaySettingFilterState.generated.h"

class UGameplaySetting;

/**
 * @brief Represents the current filter configuration for a settings screen
 * 
 * FGameplaySettingFilterState is used to determine which settings should be visible
 * and available in the UI. It supports filtering by:
 * - Search text (using FTextFilterExpressionEvaluator)
 * - Explicit allow lists
 * - Visibility/Enabled state
 * - Resettable state
 * - Inclusion of nested pages
 */
USTRUCT()
struct GAMEPLAYCOMMONSETTINGS_API FGameplaySettingFilterState
{
	GENERATED_BODY()

public:
	/** @brief Whether to include/show settings that are currently disabled */
	UPROPERTY()
	uint8 bIncludeDisabled:1;

	/** @brief Whether to include/show settings that are currently hidden */
	UPROPERTY()
	uint8 bIncludeHidden:1;

	/** @brief Whether to include/show settings that are not resettable to default */
	UPROPERTY()
	uint8 bIncludeResetable:1;
	
	/** @brief Whether to include/show nested sub-pages in the filtered results */
	UPROPERTY()
	uint8 bIncludeNestedPages:1;

public:
	FGameplaySettingFilterState();

	/**
	 * @brief Sets the search text for filtering settings
	 * @param InSearchText The text to search for
	 */
	void SetSearchText(const FString& InSearchText);

	/**
	 * @brief Evaluates whether a given setting passes the current filter
	 * @param InSetting The setting to check
	 * @return True if the setting passes the filter
	 */
	bool DoesSettingPassFilter(const UGameplaySetting& InSetting) const;

	/**
	 * @brief Adds a setting to the root list (usually top-level categories)
	 * @param InSetting The setting to add
	 */
	void AddSettingToRootList(UGameplaySetting* InSetting);
	
	/**
	 * @brief Adds a setting to the explicit allow list
	 * @param InSetting The setting to allow
	 */
	void AddSettingToAllowList(UGameplaySetting* InSetting);

	/**
	 * @brief Checks if a setting is in the explicit allow list
	 * @param InSetting The setting to check
	 * @return True if allowed
	 */
	bool IsSettingInAllowList(const UGameplaySetting* InSetting) const
	{
		return SettingAllowList.Contains(InSetting);
	}

	/** @brief Gets the collection of settings in the root list */
	const TArray<UGameplaySetting*>& GetSettingRootList() const { return (const TArray<UGameplaySetting*>&)SettingRootList; }
	
	/**
	 * @brief Checks if a setting is in the root list
	 * @param InSetting The setting to check
	 * @return True if in root list
	 */
	bool IsSettingInRootList(const UGameplaySetting* InSetting) const
	{
		return SettingRootList.Contains(InSetting);
	}

private:
	/** Evaluator used for complex text-based searching */
	FTextFilterExpressionEvaluator SearchTextEvaluator;

	/** The list of root settings */
	UPROPERTY()
	TArray<TObjectPtr<UGameplaySetting>> SettingRootList;

	/** If this is non-empty, then only settings in here are allowed */
	UPROPERTY()
	TArray<TObjectPtr<UGameplaySetting>> SettingAllowList;
};