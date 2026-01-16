// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class ULocalPlayer;

/**
 * @brief Abstract base class for setting data sources
 * 
 * FGameplaySettingDataSource provides an abstraction layer for getting and setting
 * values from various sources. This allows settings to read/write data from:
 * - Configuration files (ini files)
 * - User settings objects
 * - Console variables
 * - Custom data sources
 * 
 * Data sources use a string-based interface for maximum flexibility, allowing
 * derived classes to convert between string representations and their native types.
 * 
 * @note This is an abstract class. Use FGameplaySettingDataSourceDynamic or implement your own.
 * @see FGameplaySettingDataSourceDynamic
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingDataSource : public TSharedFromThis<FGameplaySettingDataSource>
{
public:
	virtual ~FGameplaySettingDataSource() { }

	/**
	 * @brief Initializes the data source asynchronously
	 * 
	 * Some settings may require asynchronous initialization (e.g., loading from disk,
	 * network requests). The settings system will wait for all data sources to complete
	 * startup before displaying the settings.
	 * 
	 * @param InLocalPlayer The local player this data source belongs to
	 * @param StartupCompleteCallback Callback to execute when startup is complete
	 */
	virtual void Startup(ULocalPlayer* InLocalPlayer, FSimpleDelegate StartupCompleteCallback) { StartupCompleteCallback.ExecuteIfBound(); }
	
	/**
	 * @brief Resolves the data source to ensure it's ready for use
	 * 
	 * @param InLocalPlayer The local player context
	 * @return True if the data source was resolved successfully
	 */
	virtual bool Resolve(ULocalPlayer* InLocalPlayer) = 0;
	
	/**
	 * @brief Gets the current value as a string
	 * 
	 * @param InLocalPlayer The local player context
	 * @return The value as a string representation
	 */
	virtual FString GetValueAsString(ULocalPlayer* InLocalPlayer) const = 0;
	
	/**
	 * @brief Sets the value from a string
	 * 
	 * @param InLocalPlayer The local player context
	 * @param Value The new value as a string
	 */
	virtual void SetValue(ULocalPlayer* InLocalPlayer, const FString& Value) = 0;
	
	/**
	 * @brief Gets a string representation of this data source for debugging
	 * @return String describing this data source
	 */
	virtual FString ToString() const = 0;
};