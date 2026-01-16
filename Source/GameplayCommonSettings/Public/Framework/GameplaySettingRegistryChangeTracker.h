// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/ObjectKey.h"

enum class EGameplaySettingChangeReason : uint8;

class UGameplaySetting;
class UGameplaySettingRegistry;

/**
 * @brief Tracks changes made to settings within a registry
 * 
 * FGameplaySettingRegistryChangeTracker listens for change events from a registry
 * and keeps track of which settings have been modified. It provides functionality
 * to apply all pending changes, restore modified settings to their initial values,
 * and check if any settings are currently "dirty" (changed but not applied).
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingRegistryChangeTracker : public FNoncopyable
{
public:
	/** @brief Constructs a new change tracker */
	FGameplaySettingRegistryChangeTracker();
	
	/** @brief Destructs the change tracker and stops watching the registry */
	~FGameplaySettingRegistryChangeTracker();

	/**
	 * @brief Begins watching a settings registry for changes
	 * @param InRegistry The registry to observe
	 */
	void WatchRegistry(UGameplaySettingRegistry* InRegistry);
	
	/** @brief Stops watching the currently tracked registry */
	void StopWatchRegistry();

	/** @brief Applies all pending changes to the settings in the registry */
	void ApplyChanges();
	
	/** @brief Reverts all modified settings back to their initial values */
	void RestoreToInitial();
	
	/** @brief Clears the internal list of dirty settings without applying or reverting them */
	void ClearDirtyState();

	/** @brief Gets the number of settings that have been changed */
	int32 GetDirtyCount() const { return DirtySettings.Num(); }

	/** @brief Checks if settings are currently being restored */
	bool IsRestoringSettings() const { return bRestoringSettings; }
	
	/** @brief Checks if any settings in the registry have been modified */
	bool HaveSettingsBeenChanged() const { return bSettingsChanged; }

private:
	/** Internal handler for setting change events */
	void HandleSettingChanged(UGameplaySetting* Setting, EGameplaySettingChangeReason Reason);

	/** Whether any settings have been changed */
	bool bSettingsChanged;
	
	/** Whether a restore operation is currently in progress */
	bool bRestoringSettings;

	/** The registry being watched */
	TWeakObjectPtr<UGameplaySettingRegistry> Registry;
	
	/** Mapping of dirty settings indexed by their object keys */
	TMap<FObjectKey, TWeakObjectPtr<UGameplaySetting>> DirtySettings;
};