// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FGameplayCommonUIModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
