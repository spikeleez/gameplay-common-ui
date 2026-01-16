// Copyright Spike Plugins 2026. All Rights Reserved.

#include "GameplayCommonUI.h"
#include "GameplayTagsManager.h"
#include "Misc/Paths.h"

void FGameplayCommonUIModule::StartupModule()
{
	UGameplayTagsManager::Get().AddTagIniSearchPath(FPaths::ProjectPluginsDir() / TEXT("GameplayCommonUI/Configs/Tags"));
}

void FGameplayCommonUIModule::ShutdownModule()
{
	
}

IMPLEMENT_MODULE(FGameplayCommonUIModule, GameplayCommonUI)
