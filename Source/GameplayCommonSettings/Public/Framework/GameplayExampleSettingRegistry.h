// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplaySettingRegistry.h"
#include "GameplayExampleSettingRegistry.generated.h"

class UGameplaySettingCollection;

#define GET_SHARED_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName) \
		GET_SETTINGS_FUNCTION_PATH(UGameplaySettingsLocalPlayer, GetSharedSettings, UGameplaySettingsShared, FunctionOrPropertyName)

#define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName) \
		GET_SETTINGS_FUNCTION_PATH(UGameplaySettingsLocalPlayer, GetLocalSettings, UGameplaySettingsLocal, FunctionOrPropertyName)

UCLASS(MinimalAPI)
class UGameplayExampleSettingRegistry : public UGameplaySettingRegistry
{
	GENERATED_BODY()
	
public:
	UGameplayExampleSettingRegistry();

	//~Begin UGameplaySettingRegistry interface
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) override;
	//~End of UGameplaySettingRegistry interface
	
protected:
	//============================================================================
	// VIDEO SETTINGS
	//============================================================================
	UGameplaySettingCollection* InitializeVideoSettings(ULocalPlayer* InLocalPlayer);
	void InitializeVideoSettings_FrameRates(UGameplaySettingCollection* Screen, ULocalPlayer* InLocalPlayer);
	void AddPerformanceStatPage(UGameplaySettingCollection* PerfStatsOuterCategory, ULocalPlayer* InLocalPlayer);

	//============================================================================
	// AUDIO SETTINGS
	//============================================================================
	UGameplaySettingCollection* InitializeAudioSettings(ULocalPlayer* InLocalPlayer);

	//============================================================================
	// GAMEPLAY SETTINGS
	//============================================================================
	UGameplaySettingCollection* InitializeGameplaySettings(ULocalPlayer* InLocalPlayer);

	//============================================================================
	// MOUSE & KEYBOARD SETTINGS
	//============================================================================
	UGameplaySettingCollection* InitializeMouseAndKeyboardSettings(ULocalPlayer* InLocalPlayer);

	//============================================================================
	// GAMEPAD SETTINGS
	//============================================================================
	UGameplaySettingCollection* InitializeGamepadSettings(ULocalPlayer* InLocalPlayer);
	
protected:
	UPROPERTY()
	TObjectPtr<UGameplaySettingCollection> VideoSettings;
	
	UPROPERTY()
	TObjectPtr<UGameplaySettingCollection> AudioSettings;

	UPROPERTY()
	TObjectPtr<UGameplaySettingCollection> GameplaySettings;

	UPROPERTY()
	TObjectPtr<UGameplaySettingCollection> MouseAndKeyboardSettings;

	UPROPERTY()
	TObjectPtr<UGameplaySettingCollection> GamepadSettings;
};
