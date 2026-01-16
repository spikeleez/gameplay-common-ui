// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Subsystems/GameplayCommonExtensionSubsystem.h"
#include "GameplayCommonExtensionLibrary.generated.h"

/**
 * @brief Blueprint function library for the UI Extension system
 * 
 * Provides static utility methods for manual management of extension and 
 * extension point handles, such as validation and unregistration.
 */
UCLASS()
class GAMEPLAYCOMMONUI_API UGameplayCommonExtensionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UGameplayCommonExtensionLibrary() { }
	
	/**
	 * @brief Unregisters an extension using its handle
	 * @param Handle The handle identifying the extension to remove
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Extension Library")
	static void UnregisterExtension(UPARAM(ref) FGameplayUIExtensionHandle& Handle);

	/**
	 * @brief Checks if an extension handle is currently valid
	 * @param Handle The handle to check
	 * @return True if the extension is registered and active
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Extension Library")
	static bool IsValidExtension(UPARAM(ref) FGameplayUIExtensionHandle& Handle);
	
	/**
	 * @brief Unregisters an extension point using its handle
	 * @param Handle The handle identifying the extension point to remove
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Extension Library")
	static void UnregisterExtensionPoint(UPARAM(ref) FGameplayUIExtensionPointHandle& Handle);

	/**
	 * @brief Checks if an extension point handle is currently valid
	 * @param Handle The handle to check
	 * @return True if the extension point is registered and active
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Extension Library")
	static bool IsValidExtensionPoint(UPARAM(ref) FGameplayUIExtensionPointHandle& Handle);
};

