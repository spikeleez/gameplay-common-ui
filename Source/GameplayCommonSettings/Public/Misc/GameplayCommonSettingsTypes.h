// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplayCommonSettingsTypes.generated.h"

UENUM(BlueprintType)
enum class EGameplayColorBlindMode : uint8
{
	Off,
	
	// Deuteranope (green weak/blind)
	Deuteranope,
	
	// Protanope (red weak/blind)
	Protanope,
	
	// Tritanope (blue weak/blind)
	Tritanope,
	
	MAX UMETA(Hidden)
	
};

UENUM(BlueprintType)
enum class EGameplayAllowBackgroundAudio : uint8
{
	Off,
	AllSounds,
	
	MAX UMETA(Hidden)
};
