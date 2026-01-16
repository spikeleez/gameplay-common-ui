// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplaySetting.h"
#include "GameplaySettingValue.generated.h"

/**
 * @brief Base class for settings that represent a configurable value
 * 
 * UGameplaySettingValue is the abstract base class for all settings that can be
 * changed by the user. It provides functionality to store, reset, and restore values.
 * 
 * This class introduces the concept of:
 * - **Initial Value**: The value when the settings screen is opened
 * - **Default Value**: The factory/baseline value
 * - **Current Value**: The active value being used
 * 
 * Derived classes implement the actual value storage and manipulation logic
 * for specific types (discrete, scalar, etc.).
 * 
 * @note This is an abstract class. Use UGameplaySettingValueDiscrete or UGameplaySettingValueScalar instead.
 * @see UGameplaySettingValueDiscrete, UGameplaySettingValueScalar, UGameplaySetting
 */
UCLASS(Abstract)
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValue : public UGameplaySetting
{
	GENERATED_BODY()

public:
	UGameplaySettingValue(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * @brief Stores the current value as the initial value
	 * 
	 * This is called during initialization and should also be called when
	 * applying settings. The initial value serves as the restore point when
	 * the user wants to discard changes.
	 */
	virtual void StoreInitial() PURE_VIRTUAL(UGameplaySettingValue::StoreInitial, );
	
	/**
	 * @brief Resets the setting to its default value
	 * 
	 * Restores the setting to the factory default value. This is typically
	 * used when the user wants to reset all settings to their original state.
	 */
	virtual void ResetToDefault() PURE_VIRTUAL(UGameplaySettingValue::ResetToDefault, );

	/**
	 * @brief Restores the setting to its initial value
	 * 
	 * Reverts the setting to the value it had when the settings screen was opened.
	 * This allows users to discard changes without applying them.
	 */
	virtual void RestoreToInitial() PURE_VIRTUAL(UGameplaySettingValue::RestoreToInitial, );

protected:
	// ~Begin UGameplaySetting interface
	virtual void OnInitialized() override;
	// ~End of UGameplaySetting interface
};


