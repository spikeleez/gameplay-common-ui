// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CommonRotator.h"
#include "GameplaySettingRotator.generated.h"

/**
 * @brief A specialized rotator widget for game settings
 * 
 * Extends the basic CommonRotator to support specifying a default option index, 
 * which can be used to visually highlight the "recommended" or "default" choice.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingRotator : public UCommonRotator
{
	GENERATED_BODY()

public:
	UGameplaySettingRotator(const FObjectInitializer& ObjectInitializer);

	/** @brief Sets the index representing the default setting value */
	void SetDefaultOption(int32 DefaultOptionIndex);

protected:
	/** @brief Blueprint event triggered when a default option is specified */
	UFUNCTION(BlueprintImplementableEvent, Category="Events", meta=(DisplayName = "On Default Option Specified"))
	void BP_OnDefaultOptionSpecified(int32 DefaultOptionIndex);
};