// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameFramework/GameUserSettings.h"
#include "Misc/GameplayCommonPerformanceSettings.h"
#include "GameplaySettingsLocal.generated.h"


class USoundControlBusMix;
class USoundControlBus;
/**
 * @brief Helper struct to store a snapshot of scalability settings.
 * 
 * Used to revert settings or compare against defaults.
 */
USTRUCT()
struct FGameplayScalabilitySnapshot
{
	GENERATED_BODY()

	FGameplayScalabilitySnapshot();

	/** The stored quality levels */
	Scalability::FQualityLevels Qualities;

	/** Whether the snapshot is currently active and filled with data */
	bool bActive = false;
	
	/** Whether the snapshot contains overridden settings */
	bool bHasOverrides = false;
};

/**
 * @brief Manages local, non-replicated game settings for the user.
 * 
 * This class extends UGameUserSettings to add project-specific settings like:
 * - Complex frame rate limiting (Menu, Background, Battery)
 * - Display gamma
 * - Performance stat HUD visibility
 * - Latency tracking (Reflex)
 * - Input controller branding
 */
UCLASS(Config = GameUserSettings)
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingsLocal : public UGameUserSettings
{
	GENERATED_BODY()

public:
	UGameplaySettingsLocal();

	/** @brief Gets the singleton instance of the local game settings. */
	static UGameplaySettingsLocal* Get();

	// ~Begin UObject interface
	virtual void BeginDestroy() override;
	// ~End of UObject interface

	// ~Begin UGameUserSettings interface
	virtual void SetToDefaults() override;
	virtual void LoadSettings(bool bForceReload) override;
	virtual void ConfirmVideoMode() override;
	virtual float GetEffectiveFrameRateLimit() override;
	virtual void ResetToCurrentSettings() override;
	virtual void ApplyNonResolutionSettings() override;
	virtual int32 GetOverallScalabilityLevel() const override;
	virtual void SetOverallScalabilityLevel(int32 Value) override;
	// ~End of UGameUserSettings interface

	//=============================================================================
	// DISPLAY & GRAPHICS
	//=============================================================================
public:
	/**
	 * @brief Sets the display gamma (brightness)
	 * @param InValue The gamma value. Standard is usually 2.2.
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Display")
	void SetDisplayGamma(float InValue);

	/**
	 * @brief Gets the current display gamma
	 * @return The gamma value.
	 */
	UFUNCTION(BlueprintPure, Category = "Settings|Display")
	float GetDisplayGamma() const;

private:
	/** Internal helper to apply the gamma value to the engine. */
	void ApplyDisplayGamma();

private:
	/** The user-configured display gamma. */
	UPROPERTY(Config) 
	float DisplayGamma = 2.2f;

	//=============================================================================
	// SCALABILITY & BENCHMARKING
	//=============================================================================

public:
	/** @brief Applies all pending scalability settings to the system. */
	virtual void ApplyScalabilitySettings();

	/**
	 * @brief Returns true if this platform allows running the auto benchmark
	 * @return True if benchmarking is supported
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Scalability")
	bool CanRunAutoBenchmark() const;

	/**
	 * @brief Returns true if we should run the auto benchmark (e.g. first launch)
	 * @return True if a benchmark is recommended
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Scalability")
	bool ShouldRunAutoBenchmarkAtStartup() const;

	/**
	 * @brief Runs the auto benchmark to detect hardware capabilities
	 * @param bSaveImmediately If true, saves the detected settings to config right away
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Scalability")
	void RunAutoBenchmark(bool bSaveImmediately);

	/** @brief Returns the first frame rate at which overall quality is restricted by the device profile. */
	int32 GetFirstFrameRateWithQualityLimit() const;

	/** @brief Returns the lowest quality at which there's a limit on the overall frame rate (-1 if no limit). */
	int32 GetLowestQualityWithFrameRateLimit() const;

	/** @brief Returns the maximum supported overall quality level (e.g., 0-3). */
	int32 GetMaxSupportedOverallQualityLevel() const;

private:
	/** Snapshot of the device's default scalability settings upon initialization. */
	FGameplayScalabilitySnapshot DeviceDefaultScalabilitySettings;

	/** Guard to prevent recursion when setting overall quality. */
	bool bSettingOverallQualityGuard = false;

	//=============================================================================
	// FRAME RATE LIMITS
	//=============================================================================

public:
	/** @brief Gets the frame rate limit when on battery */
	UFUNCTION(BlueprintCallable, Category = "Settings|Frame Rate")
	float GetFrameRateLimit_OnBattery() const;

	/** @brief Sets the frame rate limit when on battery */
	UFUNCTION(BlueprintCallable, Category = "Settings|Frame Rate")
	void SetFrameRateLimit_OnBattery(float NewLimitFPS);

	/** @brief Gets the frame rate limit when in menus */
	UFUNCTION(BlueprintCallable, Category = "Settings|Frame Rate")
	float GetFrameRateLimit_InMenu() const;

	/** @brief Sets the frame rate limit when in menus */
	UFUNCTION(BlueprintCallable, Category = "Settings|Frame Rate")
	void SetFrameRateLimit_InMenu(float NewLimitFPS);

	/** @brief Gets the frame rate limit when the app is in the background */
	UFUNCTION(BlueprintCallable, Category = "Settings|Frame Rate")
	float GetFrameRateLimit_WhenBackgrounded() const;

	/** @brief Sets the frame rate limit when the app is in the background */
	UFUNCTION(BlueprintCallable, Category = "Settings|Frame Rate")
	void SetFrameRateLimit_WhenBackgrounded(float NewLimitFPS);

	/** @brief Gets the global frame rate limit. */
	UFUNCTION(BlueprintCallable, Category = "Settings|Frame Rate")
	float GetFrameRateLimit_Always() const;

	/** @brief Sets the global frame rate limit. */
	UFUNCTION(BlueprintCallable, Category = "Settings|Frame Rate")
	void SetFrameRateLimit_Always(float NewLimitFPS);

	/** @brief Sets whether we are currently in the frontend (affects performance settings). */
	void SetShouldUseFrontendPerformanceSettings(bool bInFrontEnd);

protected:
	/** Checks if we should use frontend-specific performance settings. */
	bool ShouldUseFrontendPerformanceSettings() const;

	/** Updates the engine's max FPS based on the current state (Menu, Battery, etc). */
	void UpdateEffectiveFrameRateLimit();

	/** Callback when the application activation state changes (focus/background). */
	void OnAppActivationStateChanged(bool bIsActive);

private:
	/** Frame rate limit when the device is running on battery. */
	UPROPERTY(Config)
	float FrameRateLimit_OnBattery;

	/** Frame rate limit when the game is in a menu or UI context. */
	UPROPERTY(Config)
	float FrameRateLimit_InMenu;

	/** Frame rate limit when the game window is not in focus (backgrounded). */
	UPROPERTY(Config)
	float FrameRateLimit_WhenBackgrounded;

	/** True if the player is currently in the frontend/main menu. */
	bool bInFrontEndForPerformancePurposes = false;

	/** Handle for the application activation delegate. */
	FDelegateHandle OnApplicationActivationStateChangedHandle;

	//=============================================================================
	// PERFORMANCE STATS (HUD)
	//=============================================================================
public:
	/** @brief Returns the display mode for the specified performance stat. */
	EGameplayPerformanceStatDisplayMode GetPerfStatDisplayState(EGameplayDisplayablePerformanceStat Stat) const;
	
	/** @brief Sets the display mode for the specified performance stat. */
	void SetPerfStatDisplayState(EGameplayDisplayablePerformanceStat Stat, EGameplayPerformanceStatDisplayMode DisplayMode);

	/** @brief Event fired when a performance stat display setting changes. */
	DECLARE_EVENT(UGameplaySettingsLocal, FPerfStatSettingsChanged);
	/** @brief Accessor for the performance stat change event */
	FPerfStatSettingsChanged& OnPerfStatDisplayStateChanged() { return PerfStatSettingsChangedEvent; }

private:
	/** Config map of stats to display modes. */
	UPROPERTY(Config)
	TMap<EGameplayDisplayablePerformanceStat, EGameplayPerformanceStatDisplayMode> DisplayStatList;

	/** Delegate instance for perf stat changes. */
	FPerfStatSettingsChanged PerfStatSettingsChangedEvent;

	//=============================================================================
	// LATENCY (NVIDIA REFLEX / FLASH)
	//=============================================================================
public:
	/** @brief Checks if the platform supports latency flash indicators. */
	static bool DoesPlatformSupportLatencyMarkers();
	
	/** @brief Enables or disables the latency flash indicators. */
	UFUNCTION()
	void SetEnableLatencyFlashIndicators(const bool bNewVal);

	/** @brief Returns true if latency flash indicators are enabled. */
	UFUNCTION()
	bool GetEnableLatencyFlashIndicators() const { return bEnableLatencyFlashIndicators; }

	/** @brief Event fired when latency flash indicator setting changes. */
	DECLARE_EVENT(UGameplaySettingsLocal, FLatencyFlashInidicatorSettingChanged);
	/** @brief Accessor for the latency flash indicator change event */
	FLatencyFlashInidicatorSettingChanged& OnLatencyFlashIndicatorSettingsChangedEvent() { return LatencyFlashIndicatorSettingsChangedEvent; }

	/** @brief Checks if the platform supports latency tracking stats. */
	static bool DoesPlatformSupportLatencyTrackingStats();
	
	/** @brief Enables or disables latency tracking stats. */
	UFUNCTION()
	void SetEnableLatencyTrackingStats(const bool bNewVal);

	/** @brief Returns true if latency tracking stats are enabled. */
	UFUNCTION()
	bool GetEnableLatencyTrackingStats() const { return bEnableLatencyTrackingStats; }

	/** @brief Event fired when latency stat tracking setting changes. */
	DECLARE_EVENT(UGameplaySettingsLocal, FLatencyStatEnabledSettingChanged);
	/** @brief Accessor for the latency stat enabled change event */
	FLatencyStatEnabledSettingChanged& OnLatencyStatIndicatorSettingsChangedEvent() { return LatencyStatIndicatorSettingsChangedEvent; }

private:
	/** Helper to apply the latency tracking stat setting to the engine module. */
	void ApplyLatencyTrackingStatSetting();

private:
	/** If true, enable latency flash markers (used to measure input latency via hardware). */
	UPROPERTY(Config)
	bool bEnableLatencyFlashIndicators = false;

	/** If true, the game will track latency stats (e.g. NVIDIA Reflex). */
	UPROPERTY(Config)
	bool bEnableLatencyTrackingStats;

	/** Delegate instance for flash indicator changes. */
	FLatencyFlashInidicatorSettingChanged LatencyFlashIndicatorSettingsChangedEvent;

	/** Delegate instance for latency stat changes. */
	FLatencyStatEnabledSettingChanged LatencyStatIndicatorSettingsChangedEvent;

	//=============================================================================
	// DEVICE PROFILES
	//=============================================================================
protected:
	/** Handles logic when device profiles might have changed. */
	void ReapplyThingsDueToPossibleDeviceProfileChange();

private:
	/** Suffix desired by the user (saved in config). */
	UPROPERTY(Config)
	FString UserChosenDeviceProfileSuffix;

	/** Temporary suffix we are trying to apply. */
	UPROPERTY(Transient)
	FString DesiredUserChosenDeviceProfileSuffix;

	/** The currently active suffix applied to the device profile. */
	UPROPERTY(Transient)
	FString CurrentAppliedDeviceProfileOverrideSuffix;

	//=============================================================================
	// INPUT & CONTROLLERS
	//=============================================================================
public:
	/**
	 * @brief Sets the controller platform (e.g. XBox, PS5) for UI icons/layouts. 
	 * @param InControllerPlatform The name of the controller platform.
	 */
	UFUNCTION(BlueprintCallable, Category = "Settings|Controller")
	void SetControllerPlatform(const FName InControllerPlatform);

	/**
	 * @brief Gets the current controller platform name
	 * @return The platform name (e.g., PS5, Keyboard)
	 */
	UFUNCTION(BlueprintPure, Category = "Settings|Controller")
	FName GetControllerPlatform() const;

private:
	/**Maps to UCommonInputBaseControllerData name. 
	 * Found in <Platform>Game.ini under +ControllerData=...
	 */
	UPROPERTY(Config)
	FName ControllerPlatform;

	/** The specific controller preset (layout) selected. */
	UPROPERTY(Config)
	FName ControllerPreset = TEXT("Default");

	/** The name of the input config selected by the user. */
	UPROPERTY(Config)
	FName InputConfigName = TEXT("Default");
	
	//=============================================================================
	// AUDIO
	//=============================================================================
public:
	DECLARE_EVENT_OneParam(UGameplaySettingsLocal, FAudioDeviceChanged, const FString& /*DeviceId*/);
	FAudioDeviceChanged OnAudioOutputDeviceChanged;
	
	/** Returns if we're using High Dynamic Range Audio mode (HDR Audio) **/
	UFUNCTION()
	bool GetHDRAudioMode() const { return bUseHDRAudioMode; }

	/** Enables or disables High Dynamic Range Audio mode (HDR Audio) */
	UFUNCTION()
	void SetHDRAudioMode(bool bEnabled);
	
	/** Returns if we're using headphone mode (HRTF) **/
	UFUNCTION()
	bool IsHeadphoneModeEnabled() const { return bUseHeadphoneMode; }
	
	UFUNCTION()
	bool GetDesiredHeadphoneMode() const { return bDesiredHeadphoneMode; }
	
	UFUNCTION()
	void SetDesiredHeadphoneMode(bool bEnabled);
	
	/** Enables or disables headphone mode (HRTF) - NOTE this setting will be overruled if au.DisableBinauralSpatialization is set */
	UFUNCTION()
	void SetHeadphoneModeEnabled(bool bEnabled);

	/** Returns if we can enable/disable headphone mode (i.e., if it's not forced on or off by the platform) */
	UFUNCTION()
	bool CanModifyHeadphoneModeEnabled() const;
	
	UFUNCTION() 
	float GetOverallVolume() const;
	UFUNCTION() 
	void SetOverallVolume(float NewVolume);
	
	UFUNCTION() 
	float GetMusicVolume() const;
	UFUNCTION() 
	void SetMusicVolume(float NewVolume);
	
	UFUNCTION() 
	float GetSoundEffectsVolume() const;
	UFUNCTION() 
	void SetSoundEffectsVolume(float NewVolume);
	
	UFUNCTION() 
	float GetDialogueVolume() const;
	UFUNCTION() 
	void SetDialogueVolume(float NewVolume);
	
	UFUNCTION() 
	float GetVoiceChatVolume() const;
	UFUNCTION() 
	void SetVoiceChatVolume(float NewVolume);
	
	UFUNCTION()
	FString GetAudioOutputDeviceId() const { return AudioOutputDeviceId; }
	
	UFUNCTION()
	void SetAudioOutputDeviceId(const FString& NewAudioOutputDeviceId);
	
private:
	/** Whether we *want* to use headphone mode (HRTF); may or may not actually be applied **/
	UPROPERTY(Transient)
	bool bDesiredHeadphoneMode;
	
	/** Whether to use headphone mode (HRTF) **/
	UPROPERTY(config)
	bool bUseHeadphoneMode;
	
	/** Whether to use High Dynamic Range Audio mode (HDR Audio) **/
	UPROPERTY(config)
	bool bUseHDRAudioMode;
	
	UPROPERTY(Config)
	FString AudioOutputDeviceId;
	
	UPROPERTY(Config)
	float OverallVolume = 1.0f;
	
	UPROPERTY(Config)
	float MusicVolume = 1.0f;
	
	UPROPERTY(Config)
	float SoundEffectsVolume = 1.0f;
	
	UPROPERTY(Config)
	float DialogueVolume = 1.0f;
	
	UPROPERTY(Config)
	float VoiceChatVolume = 1.0f;

	UPROPERTY(Transient)
	TMap<FName/*SoundClassName*/, TObjectPtr<USoundControlBus>> ControlBusMap;

	UPROPERTY(Transient)
	TObjectPtr<USoundControlBusMix> ControlBusMix = nullptr;

	UPROPERTY(Transient)
	bool bSoundControlBusMixLoaded;
	
private:
	void LoadUserControlBusMix();
	void SetVolumeForControlBus(USoundControlBus* ControlBus, float NewVolume);
};