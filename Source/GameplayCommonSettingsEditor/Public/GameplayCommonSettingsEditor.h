// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FGameplayCommonSettingsEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override { }
    virtual void ShutdownModule() override { }
};
