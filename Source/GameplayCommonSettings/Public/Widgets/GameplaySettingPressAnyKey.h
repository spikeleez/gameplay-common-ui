// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "GameplaySettingPressAnyKey.generated.h"

/**
 * @brief An overlay widget that waits for the user to press any key or button
 * 
 * Commonly used for keybinding screens where the user needs to select a new 
 * input for an action. It uses an input pre-processor to catch any input 
 * before it reaches other widgets.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingPressAnyKey : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UGameplaySettingPressAnyKey(const FObjectInitializer& ObjectInitializer);

	/** @brief Event fired when the user successfully presses a valid key */
	DECLARE_EVENT_OneParam(UGameplaySettingPressAnyKey, FOnKeySelected, FKey);
	FOnKeySelected OnKeySelected;

	/** @brief Event fired when the user cancels the key selection (e.g., via ESC or back button) */
	DECLARE_EVENT(UGameplaySettingPressAnyKey, FOnKeySelectionCancelled);
	FOnKeySelectionCancelled OnKeySelectionCanceled;

protected:
	// ~Begin UCommonActivatableWidget interface
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	// ~End of UCommonActivatableWidget interface

	/** @brief Internal handler when the input pre-processor detects a key press */
	void HandleKeySelected(FKey InKey);

	/** @brief Internal handler when the input pre-processor detects a cancellation */
	void HandleKeySelectionCancelled();

	/** @brief Plays dismissal animations and executes a callback when finished */
	void Dismiss(TFunction<void()> PostDismissCallback);

private:
	/** Flag to prevent multiple selections during dismissal */
	bool bKeySelected = false;
	
	/** The transient input processor used to capture 'any' key */
	TSharedPtr<class FGameplayPressAnyKeyInputPreProcessor> InputProcessor;
};