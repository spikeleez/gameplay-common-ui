// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "EditCondition/GameplaySettingEditableState.h"
#include "UObject/Object.h"
#include "GameplaySetting.generated.h"

class FGameplaySettingEditCondition;
class UGameplaySettingRegistry;
class ULocalPlayer;

/**
 * @brief Specifies why a gameplay setting value changed
 * 
 * This enum is used to track the reason for setting changes, which can be useful
 * for analytics, UI updates, and understanding user behavior.
 */
UENUM(BlueprintType)
enum class EGameplaySettingChangeReason : uint8
{
	/** The setting was changed by the user or programmatically */
	Change,
	
	/** The setting changed because a dependency changed */
	DependencyChanged,
	
	/** The setting was reset to its default value */
	ResetToDefault,
	
	/** The setting was restored to its initial value */
	RestoreToInitial
};

/**
 * @brief Delegate for getting dynamic details about a setting
 * @param InLocalPlayer The local player requesting the details
 * @return Dynamic text details for the setting
 */
DECLARE_DELEGATE_RetVal_OneParam(FText, FGameplayGetSettingsDetails, ULocalPlayer& /** InLocalPlayer */);

/**
 * @brief Base class for all gameplay settings
 * 
 * UGameplaySetting represents a single configurable setting in the game.
 * Settings can be organized hierarchically, have edit conditions that control
 * their visibility and editability, and can depend on other settings.
 * 
 * Settings support:
 * - Localized display names and descriptions
 * - Dynamic visibility and editability based on conditions
 * - Analytics reporting
 * - Tagging for categorization and filtering
 * - Parent-child relationships for hierarchical organization
 * 
 * @note This is an abstract base class. Create specific setting types by deriving from this class.
 * @see UGameplaySettingRegistry, UGameplaySettingCollection, UGameplaySettingValue
 */
UCLASS(Abstract, BlueprintType)
class GAMEPLAYCOMMONSETTINGS_API UGameplaySetting : public UObject
{
	GENERATED_BODY()

public:
	/** Event broadcasted when this setting's value changes */
	DECLARE_EVENT_TwoParams(UGameplaySetting, FGameplayOnSettingChanged, UGameplaySetting* /** InSetting */, EGameplaySettingChangeReason /** InChangedReason */);
	
	/** Event broadcasted when this setting is applied */
	DECLARE_EVENT_OneParam(UGameplaySetting, FGameplayOnSettingApplied, UGameplaySetting* /** InSetting */);
	
	/** Event broadcasted when this setting's edit conditions change */
	DECLARE_EVENT_OneParam(UGameplaySetting, FGameplayOnSettingEditConditionChanged, UGameplaySetting* /** InSetting */);

	/** @brief Event triggered when the setting's value is modified */
	FGameplayOnSettingChanged OnSettingChangedEvent;

	/** @brief Event triggered when Apply() is called on the setting */
	FGameplayOnSettingApplied OnSettingAppliedEvent;

	/** @brief Event triggered when visibility or editability state is updated */
	FGameplayOnSettingEditConditionChanged OnSettingEditConditionChangedEvent;
	
public:
	UGameplaySetting(const FObjectInitializer& ObjectInitializer);
	
	/** @brief Gets the current world from the associated local player */
	virtual UWorld* GetWorld() const override;

	/**
	 * @brief Gets the non-localized developer name for this setting
	 * 
	 * The developer name is a unique identifier for this setting within its registry.
	 * It should remain constant and is used for lookup and serialization purposes.
	 * 
	 * @return The developer name of this setting
	 */
	UFUNCTION(BlueprintCallable, Category="Gameplay Setting")
	FName GetDevName() const { return DevName; }
	
	/** @brief Sets the non-localized developer name for this setting */
	void SetDevName(const FName& Value) { DevName = Value; }

	/** @brief Returns true if the list view should be scrolled/adjusted after a refresh involving this setting */
	bool GetAdjustListViewPostRefresh() const { return bAdjustListViewPostRefresh; }

	/** @brief Sets whether the list view should adjust its position post-refresh */
	void SetAdjustListViewPostRefresh(const bool Value) { bAdjustListViewPostRefresh = Value; }

	/** @brief Gets the localized display name for the UI */
	UFUNCTION(BlueprintCallable, Category="Gameplay Setting")
	FText GetDisplayName() const { return DisplayName; }

	/** @brief Sets the localized display name for the UI */
	void SetDisplayName(const FText& Value) { DisplayName = Value; }
#if !UE_BUILD_SHIPPING
	/** 
	 * @brief Sets a non-localized display name (Development/Cheats only)
	 * @note This version is for cheats and other non-shipping items, that don't need to localize their text.
	 */
	void SetDisplayName(const FString& Value) { SetDisplayName(FText::FromString(Value)); }
#endif

	/** @brief Gets whether the display name should be visible in the UI */
	UFUNCTION(BlueprintCallable, Category="Gameplay Setting")
	ESlateVisibility GetDisplayNameVisibility() const { return DisplayNameVisibility; }

	/** @brief Sets visibility for the display name label */
	void SetDisplayNameVisibility(ESlateVisibility InVisibility) { DisplayNameVisibility = InVisibility; }

	/** @brief Gets the rich text description for tooltips/detail panels */
	UFUNCTION(BlueprintCallable, Category="Gameplay Setting")
	FText GetDescriptionRichText() const { return DescriptionRichText; }

	/** @brief Sets the rich text description for tooltips/detail panels */
	void SetDescriptionRichText(const FText& Value) { DescriptionRichText = Value; }
#if !UE_BUILD_SHIPPING
	/** 
	 * @brief Sets a non-localized description (Development/Cheats only)
	 * @note This version is for cheats and other non-shipping items, that don't need to localize their text.
	 */
	void SetDescriptionRichText(const FString& Value) { SetDescriptionRichText(FText::FromString(Value)); }
#endif

	/** @brief Gets the tags associated with this setting for filtering/logic */
	UFUNCTION(BlueprintCallable, Category="Gameplay Setting")
	const FGameplayTagContainer& GetTags() const { return Tags; }

	/** @brief Adds a single gameplay tag to this setting */
	void AddTag(const FGameplayTag& TagToAdd) { Tags.AddTag(TagToAdd); }

	/** @brief Appends multiple gameplay tags to this setting */
	void AddTags(const FGameplayTagContainer& TagsToAdd) { Tags.AppendTags(TagsToAdd); }

	/** @brief Assigns the registry that owns this setting */
	void SetRegistry(UGameplaySettingRegistry* InOwningRegistry) { OwningRegistry = InOwningRegistry; }

	/** @brief Gets the searchable plain text version of the description */
	const FString& GetDescriptionPlainText() const;

	/** @brief Initializes the setting with the owning local player */
	void Initialize(ULocalPlayer* InLocalPlayer);

	/** @brief Gets the owning local player for this setting */
	ULocalPlayer* GetOwningLocalPlayer() const { return LocalPlayer; }

	/** @brief Sets a callback for providing dynamic, per-frame detail text */
	void SetDynamicDetails(const FGameplayGetSettingsDetails& InDynamicDetails) { DynamicDetails = InDynamicDetails; }

	/**
	 * @brief Gets the dynamic details about this setting
	 * 
	 * This may be information like current FPS, account numbers, or real-time 
	 * hardware stats that shouldn't be searched but should be displayed.
	 */
	UFUNCTION(BlueprintCallable, Category="Gameplay Setting")
	FText GetDynamicDetails() const;

	/** @brief Gets any warning text (e.g., "Requires Restart") associated with this setting */
	UFUNCTION(BlueprintCallable, Category="Gameplay Setting")
	FText GetWarningRichText() const { return WarningRichText; }

	/** @brief Sets warning text for this setting */
	void SetWarningRichText(const FText& Value) { WarningRichText = Value; InvalidateSearchableText(); }
#if !UE_BUILD_SHIPPING
	/** 
	 * @brief Sets non-localized warning text (Development/Cheats only)
	 * @note This version is for cheats and other non-shipping items, that don't need to localize their text.
	 */
	void SetWarningRichText(const FString& Value) { SetWarningRichText(FText::FromString(Value)); }
#endif

	/** @brief Gets the cached edit state (visible/editable/disabled) for this setting */
	const FGameplaySettingEditableState& GetEditState() const { return EditableStateCache; }

	/** @brief Adds an edit condition to control the visibility and edit-ability of this setting */
	void AddEditCondition(const TSharedRef<FGameplaySettingEditCondition>& InEditCondition);

	/** @brief Adds a dependency on another setting; if the dependency changes, this setting re-evaluates its state */
	void AddEditDependency(UGameplaySetting* InDependencySetting);

	/** @brief Sets the parent object (usually a Collection) that owns this setting */
	void SetSettingParent(UGameplaySetting* InSettingParent);

	/** @brief Gets the parent object that owns this setting */
	UGameplaySetting* GetSettingParent() const { return SettingParent; }

	/** @brief Returns true if this setting should be reported for analytics */
	bool GetIsReportedToAnalytics() const { return bReportAnalytics; }

	/** @brief Sets whether this setting captures and reports analytics data */
	void SetIsReportedToAnalytics(bool bReport) { bReportAnalytics = bReport; }

	/** @brief Gets the string value for analytics reporting */
	virtual FString GetAnalyticsValue() const { return TEXT(""); }

	/** @brief Returns true if the setting has finished any internal async initialization */
	bool IsReady() const { return bReady; }

	/** @brief Returns any child settings owned by this object (e.g. for Collections) */
	virtual TArray<UGameplaySetting*> GetChildSettings() { return TArray<UGameplaySetting*>(); }

	/**
	 * @brief Refreshes the cached editable state and optionally broadcasts notifications
	 * @param bNotifyEditConditionsChanged Whether to trigger OnSettingEditConditionChangedEvent
	 */
	void RefreshEditableState(bool bNotifyEditConditionsChanged = true);

	/**
	 * @brief Finalizes and applies any pending changes for this setting
	 * 
	 * Some settings (like Screen Resolution) may require an explicit Apply step 
	 * rather than updating immediately on every change.
	 */
	void Apply();

protected:
	/** Associated local player */
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer;

	/** Parent setting (e.g., a collection) */
	UPROPERTY(Transient)
	TObjectPtr<UGameplaySetting> SettingParent;

	/** The registry this setting belongs to */
	UPROPERTY(Transient)
	TObjectPtr<UGameplaySettingRegistry> OwningRegistry;

	/** Internal unique name */
	FName DevName;

	/** Displayed localized name */
	FText DisplayName;

	/** Visibility of the display name label */
	ESlateVisibility DisplayNameVisibility;

	/** Detailed rich text description */
	FText DescriptionRichText;

	/** Immediate warning/note text */
	FText WarningRichText;

	/** Categorization tags */
	FGameplayTagContainer Tags;

	/** Callback for dynamic detail generation */
	FGameplayGetSettingsDetails DynamicDetails;
	
	/** List of conditions controlling visibility and editability */
	TArray<TSharedRef<FGameplaySettingEditCondition>> EditConditions;

	/** Internal helper for caching culture-aware strings */
	class FGameplayStringCultureCache
	{	
	public:
		FGameplayStringCultureCache(const TFunction<FString()>& InStringGetter);

		void Invalidate();

		FString Get() const;

	private:
		mutable FString StringCache;
		mutable FCultureRef Culture;
		TFunction<FString()> StringGetter;
	};

	/** Dirty flag for search index */
	mutable bool bRefreshPlainSearchableText;

	/** Plain text cache of the description for searching */
	mutable FString AutoGenerated_DescriptionPlainText;

	/** Analytics opt-in flag */
	bool bReportAnalytics;
	
protected:
	/** @brief Called during initialization to allow one-time setup */
	virtual void Startup();

	/** @brief Marks the setting as ready for usage */
	void StartupComplete();

	/** @brief Called after initialization is complete and LocalPlayer is assigned */
	virtual void OnInitialized();

	/** @brief Internal implementation of Apply() */
	virtual void OnApply();

	/** @brief Internal implementation of editable state collection */
	virtual void OnGatherEditState(FGameplaySettingEditableState& InOutEditState) const;

	/** @brief Called when a registered dependency setting changes value */
	virtual void OnDependencyChanged();

	/** @brief Internal implementation for generating dynamic details */
	virtual FText GetDynamicDetailsInternal() const;

	/** @brief Handler for dependency change events */
	void HandleEditDependencyChanged(UGameplaySetting* DependencySetting, EGameplaySettingChangeReason Reason);

	/** @brief Handler for dependency change events (simplified) */
	void HandleEditDependencyChanged(UGameplaySetting* DependencySetting);

	/** @brief Regenerates the plain searchable text cache */
	void RefreshPlainText() const;

	/** @brief Marks the searchable text cache as dirty */
	void InvalidateSearchableText() { bRefreshPlainSearchableText = true; }

	/** @brief Broadcasts the value change event */
	void NotifySettingChanged(EGameplaySettingChangeReason Reason);

	/** @brief Internal callback for value change events */
	virtual void OnSettingChanged(EGameplaySettingChangeReason Reason);
	
	/** @brief Broadcasts the edit condition change event */
	void NotifyEditConditionsChanged();
	
	/** @brief Internal callback for edit condition change events */
	virtual void OnEditConditionsChanged();

	/** @brief Re-processes all edit conditions to determine current edit state */
	FGameplaySettingEditableState ComputeEditableState() const;

private:
	/** Readiness flag */
	bool bReady;

	/** Re-entrance guard for value changes */
	bool bOnSettingChangedEventGuard;

	/** Re-entrance guard for state changes */
	bool bOnEditConditionsChangedEventGuard;

	/** ListView management flag */
	bool bAdjustListViewPostRefresh;

	/** Current cached state of visibility and editability */
	FGameplaySettingEditableState EditableStateCache;
};

