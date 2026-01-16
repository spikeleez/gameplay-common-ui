// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Blueprint/IUserObjectListEntry.h"
#include "EditCondition/GameplaySettingEditableState.h"
#include "Framework/GameplaySettingRegistry.h"
#include "GameplaySettingListEntry.generated.h"

class UGameplayButtonBase;
class UGameplaySettingValueKeyboard;
class UGameplaySettingKeyAlreadyBound;
class UGameplaySettingPressAnyKey;
class UGameplaySettingCollectionPage;
class UGameplaySettingAction;
class UAnalogSlider;
class UGameplaySettingValueScalar;
class UCommonButtonBase;
class UGameplaySettingRotator;
class UGameplaySettingValueDiscrete;
class UCommonTextBlock;
class UGameplaySetting;
class UPanelWidget;

/**
 * @brief Abstract base class for all entries in a Gameplay Setting List
 * 
 * Provides core functionality for binding to a UGameplaySetting, handling 
 * edit condition changes, and managing focus for gamepad navigation.
 */
UCLASS(Abstract, BlueprintType, NotBlueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingListEntryBase : public UCommonUserWidget, public IUserObjectListEntry
{
	GENERATED_BODY()

public:
	/** @brief Assigns the setting object that this entry will represent */
	virtual void SetSetting(UGameplaySetting* InSetting);

	/** @brief Overrides the default display name of the setting */
	virtual void SetDisplayNameOverride(const FText& OverrideName);

protected:
	/** Flag to prevent recursive updates during value changes */
	bool bSuspendChangeUpdates = false;

	/** The custom display name to use if not empty */
	FText DisplayNameOverride = FText::GetEmpty();

	/** The setting object associated with this entry */
	UPROPERTY()
	TObjectPtr<UGameplaySetting> Setting;
	
protected:
	// ~Begin IUserObjectListEntry interface
	virtual void NativeOnEntryReleased() override;
	// ~End of IUserObjectListEntry interface
	
	/** @brief Called when the associated setting's value changes */
	virtual void OnSettingChanged();

	/** @brief Internal callback for when periodic edit condition re-evaluation is needed */
	virtual void HandleEditConditionChanged(UGameplaySetting* InSetting);

	/** @brief Updates the widget's visual state (enabled/disabled/hidden) based on current edit conditions */
	virtual void RefreshEditableState(const FGameplaySettingEditableState& InEditableState);

	// ~Begin UUserWidget interface
	virtual FReply NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent) override;
	// ~End of UUserWidget interface

	/** @brief Blueprint event to determine which sub-widget should receive focus when using a gamepad */
	UFUNCTION(BlueprintImplementableEvent, Category="List")
	UWidget* GetPrimaryGamepadFocusWidget();

private:
	/** Optional background widget for the entry */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidgetOptional, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UUserWidget> Background;
	
private:
	/** Native handler for setting change notifications */
	void HandleSettingChanged(UGameplaySetting* InSetting, EGameplaySettingChangeReason Reason);
};

/**
 * @brief Standard setting entry displaying a name and potentially a value control
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingListEntry_Setting : public UGameplaySettingListEntryBase
{
	GENERATED_BODY()

public:
	// ~Begin UGameplaySettingListEntryBase
	virtual void SetSetting(UGameplaySetting* InSetting) override;
	virtual void RefreshEditableState(const FGameplaySettingEditableState& InEditableState) override;
	// ~End of UGameplaySettingListEntryBase

private:
	/** Text block displaying the setting's name */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonTextBlock> Text_SettingName;
};

/**
 * @brief Entry for discrete settings, using a rotator and increment/decrement buttons
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingListEntry_SettingDiscrete : public UGameplaySettingListEntry_Setting
{
	GENERATED_BODY()

public:
	// ~Begin UGameplaySettingListEntry_Setting
	virtual void SetSetting(UGameplaySetting* InSetting) override;
	// ~End of UGameplaySettingListEntry_Setting

protected:
	/** Casted pointer to the discrete setting value */
	UPROPERTY()
	TObjectPtr<UGameplaySettingValueDiscrete> DiscreteSetting;
	
protected:
	// ~Begin UUserWidget
	virtual void NativeOnInitialized() override;
	// ~End of UUserWidget

	// ~Begin IUserObjectListEntry
	virtual void NativeOnEntryReleased() override;
	// ~End of IUserObjectListEntry

	/** @brief Decrements the current discrete index */
	void HandleOptionDecrease();

	/** @brief Increments the current discrete index */
	void HandleOptionIncrease();
	
	/** @brief Callback when the rotator sub-widget value changes */
	void HandleRotatorChangedValue(int32 Value, bool bUserInitiated);

	/** @brief Updates the visual state of the rotator and buttons */
	void Refresh();

	// ~Begin UGameplaySettingListEntry_Setting
	virtual void OnSettingChanged() override;
	virtual void HandleEditConditionChanged(UGameplaySetting* InSetting) override;
	virtual void RefreshEditableState(const FGameplaySettingEditableState& InEditableState) override;
	// ~End of UGameplaySettingListEntry_Setting

private:
	/** Container for the value controls */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UPanelWidget> Panel_Value;

	/** Specialized rotator widget for cycling options */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UGameplaySettingRotator> Rotator_SettingValue;

	/** Button to decrease value */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonButtonBase> Button_Decrease;

	/** Button to increase value */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonButtonBase> Button_Increase;
};

/**
 * @brief Entry for scalar settings (sliders) used for numeric ranges
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingListEntry_SettingScalar : public UGameplaySettingListEntry_Setting
{
	GENERATED_BODY()

public:
	// ~Begin UGameplaySettingListEntry_Setting
	virtual void SetSetting(UGameplaySetting* InSetting) override;
	// ~End of UGameplaySettingListEntry_Setting

protected:
	/** Casted pointer to the scalar setting value */
	UPROPERTY()
	TObjectPtr<UGameplaySettingValueScalar> ScalarSetting;
	
protected:
	/** @brief Updates the slider and value text */
	void Refresh();

	// ~Begin UUserWidget
	virtual void NativeOnInitialized() override;
	// ~End of UUserWidget

	// ~Begin IUserObjectListEntry
	virtual void NativeOnEntryReleased() override;
	// ~End of IUserObjectListEntry

	// ~Begin UGameplaySettingListEntry_Setting
	virtual void OnSettingChanged() override;
	virtual void RefreshEditableState(const FGameplaySettingEditableState& InEditableState) override;
	// ~End of UGameplaySettingListEntry_Setting

	/** @brief Internal handler for slider value changes */
	UFUNCTION()
	void HandleSliderValueChanged(float Value);

	/** @brief Callback when user stops dragging the slider */
	UFUNCTION()
	void HandleSliderCaptureEnded();

	/** @brief Blueprint event fired when value changes */
	UFUNCTION(BlueprintImplementableEvent, Category="Events", meta=(DisplayName = "On Value Changed"))
	void BP_OnValueChanged(float Value);

	/** @brief Blueprint event fired when the default value of the setting changes */
	UFUNCTION(BlueprintImplementableEvent, Category="Events", meta=(DisplayName = "On Default Value Changed"))
	void BP_OnDefaultValueChanged(float DefaultValue);

private:
	/** Container for the slider and text */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UPanelWidget> Panel_Value;

	/** The slider control */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UAnalogSlider> Slider_SettingValue;

	/** Displays the current numeric/formatted value */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonTextBlock> Text_SettingValue;
};

/**
 * @brief Entry for action-based settings, displaying a single button
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingListEntry_SettingAction : public UGameplaySettingListEntry_Setting
{
	GENERATED_BODY()

public:
	// ~Begin UGameplaySettingListEntry_Setting
	virtual void SetSetting(UGameplaySetting* InSetting) override;
	virtual void RefreshEditableState(const FGameplaySettingEditableState& InEditableState) override;
	// ~End of UGameplaySettingListEntry_Setting

protected:
	/** Casted pointer to the action setting */
	UPROPERTY()
	TObjectPtr<UGameplaySettingAction> ActionSetting;
	
protected:
	// ~Begin UUserWidget
	virtual void NativeOnInitialized() override;
	// ~End of UUserWidget

	// ~Begin IUserObjectListEntry
	virtual void NativeOnEntryReleased() override;
	// ~End of IUserObjectListEntry

	/** @brief Triggers the setting action */
	void HandleActionButtonClicked();

	/** @brief Blueprint callback when setting is first assigned */
	UFUNCTION(BlueprintImplementableEvent, Category="Events", meta=(DisplayName = "On Setting Assigned"))
	void BP_OnSettingAssigned(const FText& ActionText);

private:
	/** The button that triggers the action */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonButtonBase> Button_Action;
};

/**
 * @brief Entry for settings that navigate to another collection or sub-page
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingListEntry_SettingNavigation : public UGameplaySettingListEntry_Setting
{
	GENERATED_BODY()

public:
	// ~Begin UGameplaySettingListEntry_Setting
	virtual void SetSetting(UGameplaySetting* InSetting) override;
	virtual void RefreshEditableState(const FGameplaySettingEditableState& InEditableState) override;
	// ~End of UGameplaySettingListEntry_Setting

protected:
	/** Casted pointer to the collection/page setting */
	UPROPERTY()
	TObjectPtr<UGameplaySettingCollectionPage> CollectionSetting;
	
protected:
	// ~Begin UUserWidget
	virtual void NativeOnInitialized() override;
	// ~End of UUserWidget

	// ~Begin IUserObjectListEntry
	virtual void NativeOnEntryReleased() override;
	// ~End of IUserObjectListEntry

	/** @brief Triggers navigation to the target collection */
	void HandleNavigationButtonClicked();

	/** @brief Blueprint callback when setting is first assigned */
	UFUNCTION(BlueprintImplementableEvent, Category="Events", meta=(DisplayName = "On Setting Assigned"))
	void BP_OnSettingAssigned(const FText& ActionText);

private:
	/** The button that performs navigation */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonButtonBase> Button_Navigate;
};

UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingListEntry_KeyboardInput : public UGameplaySettingListEntry_Setting
{
	GENERATED_BODY()
	
public:
	//~Begin UGameplaySettingListEntry_Setting interface
	virtual void SetSetting(UGameplaySetting* InSetting) override;
	//~End of UGameplaySettingListEntry_Setting interface 

protected:
	//~Begin UGameplaySettingListEntry_Setting interface
	virtual void NativeOnInitialized() override;
	virtual void NativeOnEntryReleased() override;
	virtual void OnSettingChanged() override;
	//~End of UGameplaySettingListEntry_Setting interface 

	void HandlePrimaryKeyClicked();
	void HandleSecondaryKeyClicked();
	void HandleClearClicked();
	void HandleResetToDefaultClicked();

	void HandlePrimaryKeySelected(FKey InKey, UGameplaySettingPressAnyKey* PressAnyKeyPanel);
	void HandleSecondaryKeySelected(FKey InKey, UGameplaySettingPressAnyKey* PressAnyKeyPanel);
	void HandlePrimaryDuplicateKeySelected(FKey InKey, UGameplaySettingKeyAlreadyBound* DuplicateKeyPressAnyKeyPanel) const;
	void HandleSecondaryDuplicateKeySelected(FKey InKey, UGameplaySettingKeyAlreadyBound* DuplicateKeyPressAnyKeyPanel) const;
	void ChangeBinding(int32 BindSlot, const FKey& InKey);
	void HandleKeySelectionCanceled(UGameplaySettingPressAnyKey* PressAnyKeyPanel);
	void HandleKeySelectionCanceled(UGameplaySettingKeyAlreadyBound* PressAnyKeyPanel);

	void Refresh();

private:
	UPROPERTY(Transient)
	FKey OriginalKeyToBind = EKeys::Invalid;

protected:
	UPROPERTY()
	TObjectPtr<UGameplaySettingValueKeyboard> KeyboardInputSetting;

	UPROPERTY(EditDefaultsOnly, Category="Panel")
	TSubclassOf<UGameplaySettingPressAnyKey> PressAnyKeyPanelClass;

	UPROPERTY(EditDefaultsOnly, Category="Panel")
	TSubclassOf<UGameplaySettingKeyAlreadyBound> KeyAlreadyBoundWarningPanelClass;

private:	
	// Bind Widgets
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameplayButtonBase> Button_PrimaryKey;

	UPROPERTY(BlueprintReadOnly, Category="Designer", meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameplayButtonBase> Button_SecondaryKey;

	UPROPERTY(BlueprintReadOnly, Category="Designer", meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameplayButtonBase> Button_Clear;

	UPROPERTY(BlueprintReadOnly, Category="Designer", meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameplayButtonBase> Button_ResetToDefault;
};