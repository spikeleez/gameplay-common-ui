// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplaySetting.h"
#include "GameplaySettingCollection.generated.h"

struct FGameplaySettingFilterState;

/**
 * @brief Represents a collection of related gameplay settings
 * 
 * UGameplaySettingCollection is a container that groups multiple settings together.
 * It provides hierarchical organization by allowing settings to be nested within
 * collections, making it easier to organize and manage related settings.
 * 
 * Collections can contain both individual settings and other collections,
 * enabling complex setting hierarchies.
 * 
 * @note Collections are not selectable by default and usually serve as visual separators or logic groups.
 * @see UGameplaySetting, UGameplaySettingCollectionPage
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingCollection : public UGameplaySetting
{
	GENERATED_BODY()

public:
	UGameplaySettingCollection(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * @brief Gets all child settings in this collection
	 * @return Array of child settings
	 */
	virtual TArray<UGameplaySetting*> GetChildSettings() override { return Settings; }
	
	/**
	 * @brief Gets only the child collections (not individual settings)
	 * @return Array of child collections
	 */
	TArray<UGameplaySettingCollection*> GetChildCollection() const;

	/**
	 * @brief Adds a setting to this collection
	 * @param InSetting The setting to add
	 */
	void AddSetting(UGameplaySetting* InSetting);
	
	/**
	 * @brief Gets all settings that match the specified filter
	 * @param InFilterState The filter criteria
	 * @param InOutSettings Array to populate with matching settings
	 */
	virtual void GetSettingsForFilter(const FGameplaySettingFilterState& InFilterState, TArray<UGameplaySetting*>& InOutSettings) const;

	/**
	 * @brief Checks if this collection can be selected in the UI
	 * @return False for base collections (they are containers only)
	 */
	virtual bool IsSelectable() const { return false; }

protected:
	/** The settings owned by this collection */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameplaySetting>> Settings;
};

/**
 * @brief A collection that represents a navigable page of settings
 * 
 * UGameplaySettingCollectionPage extends UGameplaySettingCollection to represent
 * a navigable page in the settings UI. Unlike regular collections, pages are
 * selectable and can trigger navigation events.
 * 
 * Pages are typically used as top-level categories in settings menus,
 * such as "Graphics", "Audio", "Gameplay", etc.
 * 
 * @see UGameplaySettingCollection
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingCollectionPage : public UGameplaySettingCollection
{
	GENERATED_BODY()

public:
	/** Event broadcasted when navigation to this page is requested */
	DECLARE_EVENT_OneParam(UGameplaySettingCollectionPage, FGameplayOnExecuteNavigation, UGameplaySetting* /** Setting */);

	/** @brief Native event broadcast when ExecuteNavigation is called */
	FGameplayOnExecuteNavigation OnExecuteNavigationEvent;
	
public:
	UGameplaySettingCollectionPage(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * @brief Gets the navigation text displayed for this page
	 * @return The text shown in navigation elements
	 */
	FText GetNavigationText() const { return NavigationText; }
	
	/**
	 * @brief Sets the navigation text for this page
	 * @param Value The new navigation text
	 */
	void SetNavigationText(const FText& Value) { NavigationText = Value; }
	
#if !UE_BUILD_SHIPPING
	/**
	 * @brief Sets the navigation text from a string (non-shipping only)
	 * @param Value The navigation text as a string
	 */
	void SetNavigationText(const FString& Value) { SetNavigationText(FText::FromString(Value)); }
#endif

	// ~Begin UGameplaySetting interface
	virtual void OnInitialized() override;
	// ~End of UGameplaySetting interface

	// ~Begin UGameplaySettingCollection interface
	virtual void GetSettingsForFilter(const FGameplaySettingFilterState& InFilterState, TArray<UGameplaySetting*>& InOutSettings) const override;
	
	/** @return True as pages are intended to be clicked/selected to trigger navigation */
	virtual bool IsSelectable() const override { return true; }
	// ~End of UGameplaySettingCollection interface

	/**
	 * @brief Triggers navigation to this page
	 * 
	 * Broadcasts the OnExecuteNavigationEvent to notify listeners that
	 * this page should be navigated to.
	 */
	void ExecuteNavigation();

private:
	/** The text displayed for navigation to this page */
	FText NavigationText;
};