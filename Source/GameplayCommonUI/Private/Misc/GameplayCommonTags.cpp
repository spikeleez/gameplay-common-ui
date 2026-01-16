// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Misc/GameplayCommonTags.h"

namespace GameplayCommonTags
{
	UE_DEFINE_GAMEPLAY_TAG(Layer_Menu, "GameplayCommonUI.Layer.Menu")
	UE_DEFINE_GAMEPLAY_TAG(Layer_Game, "GameplayCommonUI.Layer.Game")
	UE_DEFINE_GAMEPLAY_TAG(Layer_GameMenu, "GameplayCommonUI.Layer.GameMenu")
	UE_DEFINE_GAMEPLAY_TAG(Layer_Modal, "GameplayCommonUI.Layer.Modal")
	
	UE_DEFINE_GAMEPLAY_TAG(Action_Escape, "UI.Action.Escape")
	
	UE_DEFINE_GAMEPLAY_TAG(Traits_PrimaryController, "Platform.Trait.Input.PrimaryController")
	UE_DEFINE_GAMEPLAY_TAG(Traits_HasStrictControllerPairing, "Platform.Trait.Input.HasStrictControllerPairing")
}
