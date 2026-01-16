// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "GameplayPerformanceStatInterface.generated.h"

/**
 * @brief Interface for objects that can provide server-side performance statistics
 */
UINTERFACE(MinimalAPI)
class UGameplayPerformanceStatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface for objects that can provide server-side performance statistics
 * 
 * Typically implemented by GameState or other globally synced objects to 
 * provide clients with information about the server's health.
 */
class GAMEPLAYCOMMONSETTINGS_API IGameplayPerformanceStatInterface
{
	GENERATED_BODY()

public:
	/** @brief Gets the current server-side frames per second */
	virtual float GetServerFPS() const { return 0.0f; }
};