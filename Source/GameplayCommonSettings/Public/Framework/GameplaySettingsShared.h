// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameFramework/SaveGame.h"
#include "Misc/GameplayCommonSettingsTypes.h"
#include "Misc/GameplaySubtitlesTypes.h"
#include "GameplaySettingsShared.generated.h"

/**
 * @brief Represents settings that are shared across different platforms or machines for a single user
 * 
 * Unlike UGameplaySettingsLocal which stores machine-specific performance settings,
 * UGameplaySettingsShared is intended for gameplay-affecting preferences that 
 * would ideally travel with the user's account (via SaveGame).
 * 
 * Examples:
 * - Subtitle preferences (Enabled, Text Size, Color, Opacity)
 * - Language/Culture preferences
 * - Accessibility settings
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingsShared : public ULocalPlayerSaveGame
{
	GENERATED_BODY()
	
public:
	virtual int32 GetLatestDataVersion() const override;
	
	/**
	 * @brief Creates a temporary, non-persisted settings object for a player
	 * @param InLocalPlayer The local player context
	 * @return The created settings object
	 */
	static UGameplaySettingsShared* CreateTemporarySettings(const ULocalPlayer* InLocalPlayer);
	
	template<typename T>
	static T* CreateTemporarySettingsTyped(const ULocalPlayer* InLocalPlayer)
	{
		return (T*)CreateTemporarySettings(InLocalPlayer);
	}
	
	/**
	 * @brief Loads existing settings or creates new ones for a player
	 * @param InLocalPlayer The local player context
	 * @return The loaded/created settings object
	 */
	static UGameplaySettingsShared* LoadOrCreateSettings(const ULocalPlayer* InLocalPlayer);
	
	template<typename T>
	static T* LoadOrCreateSettingsTyped(const ULocalPlayer* InLocalPlayer)
	{
		return (T*)LoadOrCreateSettings(InLocalPlayer);
	}
	
	/** @brief Delegate for when settings have finished loading asynchronously */
	DECLARE_DELEGATE_OneParam(FOnSettingsLoadedEvent, UGameplaySettingsShared* Settings);
	
	/**
	 * @brief Asynchronously loads or creates settings for a player
	 * @param LocalPlayer The local player context
	 * @param Delegate The callback to fire once loading is complete
	 * @return True if the load process was successfully initiated
	 */
	static bool AsyncLoadOrCreateSettings(const ULocalPlayer* LocalPlayer, FOnSettingsLoadedEvent Delegate);

	/** @brief Saves the settings to disk */
	void SaveSettings();

	/** @brief Applies the current settings to the player/engine */
	void ApplySettings();
	
private:
	/** @brief Internal helper to update a value and mark settings as dirty */
	template<typename T>
	bool ChangeValueAndDirty(T& CurrentValue, const T& NewValue)
	{
		if (CurrentValue != NewValue)
		{
			CurrentValue = NewValue;
			bIsDirty = true;
			
			return true;
		}

		return false;
	}
	
	/** Whether any settings have been modified and require saving */
	bool bIsDirty = false;
	
	//=========================================
	// SUBTITLES
	//=========================================
public:
	/** @brief Gets whether subtitles are enabled */
	UFUNCTION()
	bool GetSubtitlesEnabled() const { return bEnableSubtitles; }

	/** @brief Sets whether subtitles are enabled */
	UFUNCTION()
	void SetSubtitlesEnabled(bool Value) { ChangeValueAndDirty(bEnableSubtitles, Value); }

	/** @brief Gets the preferred text size for subtitles */
	UFUNCTION()
	EGameplaySubtitleDisplayTextSize GetSubtitlesTextSize() const { return SubtitleTextSize; }

	/** @brief Sets the preferred text size for subtitles */
	UFUNCTION()
	void SetSubtitlesTextSize(EGameplaySubtitleDisplayTextSize Value) { ChangeValueAndDirty(SubtitleTextSize, Value); ApplySubtitleOptions(); }

	/** @brief Gets the preferred text color for subtitles */
	UFUNCTION()
	EGameplaySubtitleDisplayTextColor GetSubtitlesTextColor() const { return SubtitleTextColor; }

	/** @brief Sets the preferred text color for subtitles */
	UFUNCTION()
	void SetSubtitlesTextColor(EGameplaySubtitleDisplayTextColor Value) { ChangeValueAndDirty(SubtitleTextColor, Value); ApplySubtitleOptions(); }

	/** @brief Gets the preferred text border style for subtitles */
	UFUNCTION()
	EGameplaySubtitleDisplayTextBorder GetSubtitlesTextBorder() const { return SubtitleTextBorder; }

	/** @brief Sets the preferred text border style for subtitles */
	UFUNCTION()
	void SetSubtitlesTextBorder(EGameplaySubtitleDisplayTextBorder Value) { ChangeValueAndDirty(SubtitleTextBorder, Value); ApplySubtitleOptions(); }

	/** @brief Gets the preferred background opacity for subtitles */
	UFUNCTION()
	EGameplaySubtitleDisplayBackgroundOpacity GetSubtitlesBackgroundOpacity() const { return SubtitleBackgroundOpacity; }
	
	/** @brief Sets the preferred background opacity for subtitles */
	UFUNCTION()
	void SetSubtitlesBackgroundOpacity(EGameplaySubtitleDisplayBackgroundOpacity Value) { ChangeValueAndDirty(SubtitleBackgroundOpacity, Value); ApplySubtitleOptions(); }

	/** @brief Applies current subtitle choices to the subtitle manager subsystem */
	void ApplySubtitleOptions();
	
private:
	/** Internal flag for subtitle visibility */
	UPROPERTY()
	bool bEnableSubtitles = true;

	/** Internal choice for text size */
	UPROPERTY()
	EGameplaySubtitleDisplayTextSize SubtitleTextSize = EGameplaySubtitleDisplayTextSize::Medium;

	/** Internal choice for text color */
	UPROPERTY()
	EGameplaySubtitleDisplayTextColor SubtitleTextColor = EGameplaySubtitleDisplayTextColor::White;

	/** Internal choice for text border */
	UPROPERTY()
	EGameplaySubtitleDisplayTextBorder SubtitleTextBorder = EGameplaySubtitleDisplayTextBorder::None;

	/** Internal choice for background opacity */
	UPROPERTY()
	EGameplaySubtitleDisplayBackgroundOpacity SubtitleBackgroundOpacity = EGameplaySubtitleDisplayBackgroundOpacity::Medium;
	
public:
	//=========================================
	// CULTURE & LANGUAGE
	//=========================================
	
	/** @brief Gets the pending culture that hasn't been applied yet */
	const FString& GetPendingCulture() const;

	/** @brief Sets a new culture to be applied on the next application cycle */
	void SetPendingCulture(const FString& NewCulture);

	/** @brief Callback for when the system culture has changed */
	void OnCultureChanged();

	/** @brief Discards the pending culture */
	void ClearPendingCulture();

	/** @brief Returns true if no custom culture is currently selected */
	bool IsUsingDefaultCulture() const;

	/** @brief Resets the player's language preference to the system default */
	void ResetToDefaultCulture();
	
	/** @brief Returns true if a reset to default culture is requested */
	bool ShouldResetToDefaultCulture() const { return bResetToDefaultCulture; }
	
	/** @brief Applies the selected language/culture settings to the engine */
	void ApplyCultureSettings();
	
	/** @brief Syncs the pending culture with the current engine culture */
	void ResetCultureToCurrentSettings();
	
private:
	/** The pending culture identifier (e.g., "en-US") to apply */
	UPROPERTY(Transient)
	FString PendingCulture;

	/** Transient flag indicating a revert to default culture is needed */
	bool bResetToDefaultCulture = false;
	
	//=========================================
	// COLOR BLIND
	//=========================================
public:
	UFUNCTION()
	EGameplayColorBlindMode GetColorBlindMode() const { return ColorBlindMode; }
	UFUNCTION()
	void SetColorBlindMode(EGameplayColorBlindMode NewColorBlindMode);
	
	UFUNCTION()
	int32 GetColorBlindStrength() const { return ColorBlindStrength; }
	UFUNCTION()
	void SetColorBlindStrength(int32 NewColorBlindStrength);
	
private:
	UPROPERTY()
	EGameplayColorBlindMode ColorBlindMode = EGameplayColorBlindMode::Off;
	
	UPROPERTY()
	int32 ColorBlindStrength = 10;
	
	//=========================================
	// AUDIO
	//=========================================
public:
	UFUNCTION()
	EGameplayAllowBackgroundAudio GetAllowAudioInBackground() const { return AllowAudioInBackground; }
	UFUNCTION()
	void SetAllowAudioInBackground(EGameplayAllowBackgroundAudio NewValue);
	
	void ApplyBackgroundAudioSettings();
	
private:
	UPROPERTY()
	EGameplayAllowBackgroundAudio AllowAudioInBackground = EGameplayAllowBackgroundAudio::Off;
	
	//=========================================
	// GAMEPAD AND SENSITIVITY
	//=========================================
public:
	UFUNCTION()
	double GetMouseSensitivityX() const { return MouseSensitivityX; }
	UFUNCTION()
	void SetMouseSensitivityX(double NewValue) { ChangeValueAndDirty(MouseSensitivityX, NewValue); ApplyInputSensitivity(); }
	
	UFUNCTION()
	double GetMouseSensitivityY() const { return MouseSensitivityY; }
	UFUNCTION()
	void SetMouseSensitivityY(double NewValue) { ChangeValueAndDirty(MouseSensitivityY, NewValue); ApplyInputSensitivity(); }
	
	UFUNCTION()
	bool GetInvertVerticalAxis() const { return bInvertVerticalAxis; }
	UFUNCTION()
	void SetInvertVerticalAxis(bool NewValue) { ChangeValueAndDirty(bInvertVerticalAxis, NewValue); ApplyInputSensitivity(); }
	
	UFUNCTION()
	bool GetInvertHorizontalAxis() const { return bInvertHorizontalAxis; }
	UFUNCTION()
	void SetInvertHorizontalAxis(bool NewValue) { ChangeValueAndDirty(bInvertHorizontalAxis, NewValue); ApplyInputSensitivity(); }
	
	/** Getter for gamepad move stick dead zone value. */
	UFUNCTION()
	float GetGamepadMoveStickDeadZone() const { return GamepadMoveStickDeadZone; }

	/** Setter for gamepad move stick dead zone value. */
	UFUNCTION()
	void SetGamepadMoveStickDeadZone(const float NewValue) { ChangeValueAndDirty(GamepadMoveStickDeadZone, NewValue); }

	/** Getter for gamepad look stick dead zone value. */
	UFUNCTION()
	float GetGamepadLookStickDeadZone() const { return GamepadLookStickDeadZone; }

	/** Setter for gamepad look stick dead zone value. */
	UFUNCTION()
	void SetGamepadLookStickDeadZone(const float NewValue) { ChangeValueAndDirty(GamepadLookStickDeadZone, NewValue); }
	
	void ApplyInputSensitivity();

protected:
	/** Holds the mouse horizontal sensitivity */
	UPROPERTY()
	double MouseSensitivityX = 1.0;

	/** Holds the mouse vertical sensitivity */
	UPROPERTY()
	double MouseSensitivityY = 1.0;

	/** If true then the vertical look axis should be inverted */
	UPROPERTY()
	bool bInvertVerticalAxis = false;

	/** If true then the horizontal look axis should be inverted */
	UPROPERTY()
	bool bInvertHorizontalAxis = false;
	
	/** Holds the gamepad move stick dead zone value. */
	UPROPERTY()
	float GamepadMoveStickDeadZone;

	/** Holds the gamepad look stick dead zone value. */
	UPROPERTY()
	float GamepadLookStickDeadZone;
	
	//=========================================
	// GAMEPAD VIBRATION
	//=========================================
public:
	UFUNCTION()
	bool GetTriggerHapticsEnabled() const { return bTriggerHapticsEnabled; }
	UFUNCTION()
	void SetTriggerHapticsEnabled(const bool NewValue) { ChangeValueAndDirty(bTriggerHapticsEnabled, NewValue); }

	UFUNCTION()
	bool GetTriggerPullUsesHapticThreshold() const { return bTriggerPullUsesHapticThreshold; }
	UFUNCTION()
	void SetTriggerPullUsesHapticThreshold(const bool NewValue) { ChangeValueAndDirty(bTriggerPullUsesHapticThreshold, NewValue); }

	UFUNCTION()
	uint8 GetTriggerHapticStrength() const { return TriggerHapticStrength; }
	UFUNCTION()
	void SetTriggerHapticStrength(const uint8 NewValue) { ChangeValueAndDirty(TriggerHapticStrength, NewValue); }

	UFUNCTION()
	uint8 GetTriggerHapticStartPosition() const { return TriggerHapticStartPosition; }
	UFUNCTION()
	void SetTriggerHapticStartPosition(const uint8 NewValue) { ChangeValueAndDirty(TriggerHapticStartPosition, NewValue); }
	
	UFUNCTION()
	bool GetForceFeedbackEnabled() const { return bForceFeedbackEnabled; }

	UFUNCTION()
	void SetForceFeedbackEnabled(const bool NewValue) { ChangeValueAndDirty(bForceFeedbackEnabled, NewValue); }
	
private:
	/** Is force feedback enabled when a controller is being used? */
	UPROPERTY()
	bool bForceFeedbackEnabled = true;
	
	/** Are trigger haptics enabled? */
	UPROPERTY()
	bool bTriggerHapticsEnabled = false;
	
	/** Does the game use the haptic feedback as its threshold for judging button presses? */
	UPROPERTY()
	bool bTriggerPullUsesHapticThreshold = true;
	
	/** The strength of the trigger haptic effects. */
	UPROPERTY()
	uint8 TriggerHapticStrength = 8;
	
	/** The start position of the trigger haptic effects */
	UPROPERTY()
	uint8 TriggerHapticStartPosition = 0;
};