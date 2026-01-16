// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Framework/GameplaySettingRegistry.h"
#include "GameplaySettingDetailExtension.generated.h"

class UGameplaySetting;

/**
 * @brief Base class for widgets that extend the functionality of a setting's detail view
 * 
 * Extensions are auxiliary widgets that provide additional UI elements or information 
 * for a specific setting (e.g., a "Restore Default" button, a descriptive image, 
 * or a specialized control).
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingDetailExtension : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	/** @brief Assigns the setting that this extension should represent */
	void SetSetting(UGameplaySetting* InSetting);

protected:
	/** The setting currently associated with this extension */
	UPROPERTY(Transient)
	TObjectPtr<UGameplaySetting> Setting;
	
protected:
	/** @brief Called natively when a new setting is assigned to this extension */
	virtual void NativeSettingAssigned(UGameplaySetting* InSetting);
	
	/** @brief Called natively when the value of the assigned setting changes */
	virtual void NativeSettingValueChanged(UGameplaySetting* InSetting, EGameplaySettingChangeReason Reason);

	/** @brief Blueprint event fired when a setting is assigned */
	UFUNCTION(BlueprintImplementableEvent, Category="Events", meta=(DisplayName = "On Setting Assigned"))
	void BP_OnSettingAssigned(UGameplaySetting* InSetting);

	/** @brief Blueprint event fired when the assigned setting's value changes */
	UFUNCTION(BlueprintImplementableEvent, Category="Events", meta=(DisplayName = "On Setting Value Changed"))
	void BP_OnSettingValueChanged(UGameplaySetting* InSetting);
};