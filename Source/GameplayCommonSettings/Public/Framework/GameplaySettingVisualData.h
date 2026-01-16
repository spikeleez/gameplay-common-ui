// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplaySettingVisualData.generated.h"

class UGameplaySettingListEntryBase;
class UGameplaySetting;
class UGameplaySettingDetailExtension;

/**
 * @brief Configuration for UI detail extensions based on setting class
 */
USTRUCT(BlueprintType)
struct GAMEPLAYCOMMONSETTINGS_API FGameplaySettingClassExtensions
{
	GENERATED_BODY()

public:
	/** @brief Soft references to detail extension widget classes */
	UPROPERTY(EditAnywhere, Category="Extensions")
	TArray<TSoftClassPtr<UGameplaySettingDetailExtension>> Extensions;
};

/**
 * @brief Configuration for UI detail extensions based on setting name
 */
USTRUCT(BlueprintType)
struct GAMEPLAYCOMMONSETTINGS_API FGameplaySettingNameExtensions
{
	GENERATED_BODY()

public:
	/** @brief Whether to also include extensions mapped to the setting's class */
	UPROPERTY(EditAnywhere, Category="Extensions")
	bool bIncludeClassDefaultExtensions = false;

	/** @brief Soft references to detail extension widget classes */
	UPROPERTY(EditAnywhere, Category="Extensions")
	TArray<TSoftClassPtr<UGameplaySettingDetailExtension>> Extensions;
};

/**
 * @brief Data asset that maps settings to their visual representations in the UI
 * 
 * UGameplaySettingVisualData is used to decide:
 * 1. Which UUserWidget (ListEntry) to use for a particular setting in a list.
 * 2. Which additional widgets (DetailExtensions) to show for a setting in the detail pane.
 * 
 * Mapping can be done by either the setting's C++ class or its unique FName identifier.
 */
UCLASS(BlueprintType, Blueprintable, Const, meta=(DisplayName = "Gameplay Setting Visual Data"))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingVisualData : public UDataAsset
{
	GENERATED_BODY()

public:
	UGameplaySettingVisualData(const FObjectInitializer& ObjectInitializer);

	/**
	 * @brief Gets the widget class to use for displaying a setting in a list
	 * @param InSetting The setting to look up
	 * @return The widget class, or null if no mapping found
	 */
	TSubclassOf<UGameplaySettingListEntryBase> GetEntryForSetting(UGameplaySetting* InSetting);
	
	/**
	 * @brief Gathers all detail extensions that should be displayed for a setting
	 * @param InSetting The setting to look up
	 * @return Array of extension widget classes
	 */
	virtual TArray<TSoftClassPtr<UGameplaySettingDetailExtension>> GatherDetailExtensions(UGameplaySetting* InSetting);

protected:
	/** @brief Maps setting classes to their list entry widget types */
	UPROPERTY(EditDefaultsOnly, Category="List Entries", meta=(AllowAbstract))
	TMap<TSubclassOf<UGameplaySetting>, TSubclassOf<UGameplaySettingListEntryBase>> EntryWidgetForClass;

	/** @brief Maps specific setting names to their list entry widget types (overrides class mapping) */
	UPROPERTY(EditDefaultsOnly, Category="List Entries", meta=(AllowAbstract))
	TMap<FName, TSubclassOf<UGameplaySettingListEntryBase>> EntryWidgetForName;

	/** @brief Maps setting classes to their default set of detail extensions */
	UPROPERTY(EditDefaultsOnly, Category="Extensions", meta=(AllowAbstract))
	TMap<TSubclassOf<UGameplaySetting>, FGameplaySettingClassExtensions> ExtensionsForClasses;

	/** @brief Maps specific setting names to additional detail extensions */
	UPROPERTY(EditDefaultsOnly, Category="Extensions", meta=(AllowAbstract))
	TMap<FName, FGameplaySettingNameExtensions> ExtensionsForName;
	
protected:
	/** @brief Extension point for project-specific custom entry lookup logic */
	virtual TSubclassOf<UGameplaySettingListEntryBase> GetCustomEntryForSetting(UGameplaySetting* InSetting);
};