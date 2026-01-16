// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "GameplaySettingActionInterface.generated.h"

class UGameplaySetting;

/**
 * @brief Interface for objects that can perform actions on settings
 */
UINTERFACE(MinimalAPI, BlueprintType)
class UGameplaySettingActionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * @brief Interface for objects that can perform actions on settings
 * 
 * Allows widgets or other external controllers to trigger specific logic
 * tied to a setting using GameplayTags as identifiers for the action.
 */
class GAMEPLAYCOMMONSETTINGS_API IGameplaySettingActionInterface
{
	GENERATED_BODY()

public:
	/**
	 * @brief Executes an action identified by a tag on the provided setting
	 * @param ActionTag The tag identifying the action to perform
	 * @param InSetting The setting to act upon
	 * @return True if the action was handled
	 */
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Gameplay Setting Action Interface")
	bool ExecuteActionForSetting(FGameplayTag ActionTag,  UGameplaySetting* InSetting);
};