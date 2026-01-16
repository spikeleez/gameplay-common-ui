// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Misc/GameplayCommonExtensionLibrary.h"

void UGameplayCommonExtensionLibrary::UnregisterExtension(FGameplayUIExtensionHandle& Handle)
{
	Handle.Unregister();
}

bool UGameplayCommonExtensionLibrary::IsValidExtension(FGameplayUIExtensionHandle& Handle)
{
	return Handle.IsValid();
}

void UGameplayCommonExtensionLibrary::UnregisterExtensionPoint(FGameplayUIExtensionPointHandle& Handle)
{
	Handle.Unregister();
}

bool UGameplayCommonExtensionLibrary::IsValidExtensionPoint(FGameplayUIExtensionPointHandle& Handle)
{
	return Handle.IsValid();
}
