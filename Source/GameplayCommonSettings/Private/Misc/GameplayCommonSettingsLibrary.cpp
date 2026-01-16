// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Misc/GameplayCommonSettingsLibrary.h"
#include "Interfaces/GameplayCommonSettingsInterface.h"

UGameplaySettingsShared* UGameplayCommonSettingsLibrary::GetSettingsShared(const ULocalPlayer* LocalPlayer)
{
	if (!IsValid(LocalPlayer)) return nullptr; 
	
	const IGameplayCommonSettingsInterface* CommonSettingsInterface = Cast<IGameplayCommonSettingsInterface>(LocalPlayer);
	if (!CommonSettingsInterface) return nullptr;
	
	return CommonSettingsInterface->GetSharedSettings();
}

UGameplaySettingsLocal* UGameplayCommonSettingsLibrary::GetLocalSettings(const ULocalPlayer* LocalPlayer)
{
	if (!IsValid(LocalPlayer)) return nullptr; 
	
	const IGameplayCommonSettingsInterface* CommonSettingsInterface = Cast<IGameplayCommonSettingsInterface>(LocalPlayer);
	if (!CommonSettingsInterface) return nullptr;
	
	return CommonSettingsInterface->GetLocalSettings();
}
