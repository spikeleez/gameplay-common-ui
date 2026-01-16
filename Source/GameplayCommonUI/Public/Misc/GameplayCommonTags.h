// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * @brief Namespace containing native gameplay tags for the UI system
 */
namespace GameplayCommonTags
{
	/** UI Layer for persistent background menus */
	GAMEPLAYCOMMONUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Layer_Menu)

	/** UI Layer for ingame HUD and gameplay elements */
	GAMEPLAYCOMMONUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Layer_Game)

	/** UI Layer for ingame popups and sub-menus */
	GAMEPLAYCOMMONUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Layer_GameMenu)

	/** UI Layer for modal dialogs and critical notifications */
	GAMEPLAYCOMMONUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Layer_Modal)
	
	/** Global input action for "Escape" or "Back" functionality */
	GAMEPLAYCOMMONUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Action_Escape)
	
	/** Trait tag identifying the primary local player controller */
	GAMEPLAYCOMMONUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Traits_PrimaryController)
	
	/** Trait tag indicating that input should be restricted to a specific controller pairing */
	GAMEPLAYCOMMONUI_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(Traits_HasStrictControllerPairing)
}
