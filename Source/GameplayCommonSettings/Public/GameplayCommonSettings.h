// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

#define BELOW_ENGINE_VERSION(Major, Minor)  (ENGINE_MAJOR_VERSION < (Major) || (ENGINE_MAJOR_VERSION == (Major) && ENGINE_MINOR_VERSION < (Minor)))
#define FROM_ENGINE_VERSION(Major, Minor)   !BELOW_ENGINE_VERSION(Major, Minor)

/**
 * @brief Module implementation for the Gameplay Common Settings system
 */
class FGameplayCommonSettingsModule : public IModuleInterface
{
public:
	virtual void StartupModule() override { }
	virtual void ShutdownModule() override { }
};