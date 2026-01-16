// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Framework/GameplaySettingFilterState.h"
#include "GameplaySettingPanel.generated.h"

class UGameplaySettingDetailView;
class UGameplaySettingListView;
class UGameplaySettingRegistry;
class UGameplaySetting;

/** @brief Delegate for when the currently focused (hovered/selected) setting changes */
DECLARE_MULTICAST_DELEGATE_OneParam(FOnFocusedSettingChanged, UGameplaySetting*)

/**
 * @brief Core widget for displaying a list of settings and their details
 * 
 * UGameplaySettingPanel connects a UGameplaySettingRegistry to a ListView and a 
 * DetailView. It manages:
 * - Filtering and search logic via FGameplaySettingFilterState
 * - Navigation between nested settings pages (sub-pages)
 * - Displaying detail information for the hovered/selected setting
 * - Gamepad focus orchestration
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingPanel : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UGameplaySettingPanel();
	
	// ~Begin UUserWidget interface
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
	// ~End of UUserWidget interface

	/** @brief Assigns the registry containing the settings to display */
	void SetRegistry(UGameplaySettingRegistry* InRegistry);

	/** 
	 * @brief Sets the filter for this panel, restricting which settings are visible
	 * @param InFilterState The new filter configuration
	 * @param bClearNavigationStack If true, treats this as a root-level reset of navigation
	 */
	void SetFilterState(const FGameplaySettingFilterState& InFilterState, bool bClearNavigationStack = true);

	/** @brief Gets the list of settings currently passing the filter */
	TArray<UGameplaySetting*> GetVisibleSettings() const { return (TArray<UGameplaySetting*>&)VisibleSettings; }

	/** @brief Returns true if there are sub-pages that can be popped back from */
	bool CanPopNavigationStack() const;

	/** @brief Navigates back one level in the sub-page navigation stack */
	void PopNavigationStack();

	/**
	 * @brief Gets settings that can be reset to their default values
	 * 
	 * Includes all filtered settings but excludes nested navigation entries.
	 */
	TArray<UGameplaySetting*> GetSettingsWeCanResetToDefault() const;

	/** @brief Selects a specific setting in the list by its developer name */
	void SelectSetting(const FName& SettingDevName);
	
	/** @brief Gets the currently selected setting object */
	UGameplaySetting* GetSelectedSetting() const;

	/** @brief Triggers a full refresh of the settings list UI */
	void RefreshSettingsList();

	/** Broadcaster for focused setting changes */
	FOnFocusedSettingChanged OnFocusedSettingChanged;

protected:
	/** @brief Internal helper to subscribe to registry update events */
	void RegisterRegistryEvents();

	/** @brief Internal helper to unsubscribe from registry update events */
	void UnregisterRegistryEvents();

	/** @brief Handler for entry hover events from the list view */
	void HandleSettingItemHoveredChanged(UObject* Item, bool bHovered);

	/** @brief Handler for selection changes in the list view */
	void HandleSettingItemSelectionChanged(UObject* Item);

	/** @brief Internal helper to update the detail view with a specific setting's info */
	void FillSettingDetails(UGameplaySetting* InSetting);

	/** @brief Handler for triggering actions (like Reset, Gamma test) from settings */
	void HandleSettingNamedAction(UGameplaySetting* InSetting, FGameplayTag GameSettingsActionTag);

	/** @brief Handler for entering sub-page collections */
	void HandleSettingNavigation(UGameplaySetting* InSetting);

	/** @brief Callback when a setting's edit state (visible/editable/disabled) changes */
	void HandleSettingEditConditionsChanged(UGameplaySetting* InSetting);

private:
	/** The registry being displayed */
	UPROPERTY(Transient)
	TObjectPtr<UGameplaySettingRegistry> Registry;

	/** List of setting objects currently being shown in the ListView */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameplaySetting>> VisibleSettings;

	/** Tracking the last setting focused by the user */
	UPROPERTY(Transient)
	TObjectPtr<UGameplaySetting> LastHoveredOrSelectedSetting;

	/** The current active filter */
	UPROPERTY(Transient)
	FGameplaySettingFilterState FilterState;

	/** Stack of filters used for sub-page navigation tracking */
	UPROPERTY(Transient)
	TArray<FGameplaySettingFilterState> FilterNavigationStack;

	/** Desired selection to restore after a list refresh */
	FName DesiredSelectionPostRefresh;

	/** Internal flag for scrolling management */
	bool bAdjustListViewPostRefresh = true;

private:
	/** The list view displaying entries */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameplaySettingListView> ListView_Settings;
	
	/** The detail view showing hovered/selected info */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameplaySettingDetailView> Details_Settings;

private:
	/** Delegate type for blueprint action notifications */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnExecuteNamedActionBlueprint, UGameplaySetting*, Setting, FGameplayTag, ActionTag);
	/** Blueprint-exposed event for setting actions */
	
	UPROPERTY(BlueprintAssignable, Category="Events", meta=(DisplayName = "On Execute Named Action"))
	FOnExecuteNamedActionBlueprint OnExecuteNamedActionBlueprint;

private:
	/** Handle for throttled list refreshes */
	FTSTicker::FDelegateHandle RefreshHandle;
};