// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "GameplayCommonSettingsInterface.generated.h"

class UGameplaySettingsLocal;
class UGameplaySettingsShared;

/**
 * @brief Interface for objects that provide access to game settings
 */
UINTERFACE(MinimalAPI, meta=(CannotImplementInterfaceInBlueprint))
class UGameplayCommonSettingsInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface for objects that provide access to game settings
 * 
 * Typically implemented by LocalPlayers or similar high-level objects to 
 * provide a unified way to retrieve both shared and local settings.
 */
class IGameplayCommonSettingsInterface
{
	GENERATED_BODY()
	
public:
	/** @brief Gets the shared (cross-platform) settings for this user */
	virtual UGameplaySettingsShared* GetSharedSettings() const = 0;
	
	/** @brief Gets the local (machine-specific) settings for this user */
	virtual UGameplaySettingsLocal* GetLocalSettings() const = 0;
};