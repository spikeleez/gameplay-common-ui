// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplaySettingDetailExtension.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingDetailExtension)

void UGameplaySettingDetailExtension::SetSetting(UGameplaySetting* InSetting)
{
	NativeSettingAssigned(InSetting);
}

void UGameplaySettingDetailExtension::NativeSettingAssigned(UGameplaySetting* InSetting)
{
	if (Setting)
	{
		Setting->OnSettingChangedEvent.RemoveAll(this);
	}

	Setting = InSetting;
	Setting->OnSettingChangedEvent.AddUObject(this, &UGameplaySettingDetailExtension::NativeSettingValueChanged);

	BP_OnSettingAssigned(InSetting);
}

void UGameplaySettingDetailExtension::NativeSettingValueChanged(UGameplaySetting* InSetting, EGameplaySettingChangeReason Reason)
{
	BP_OnSettingValueChanged(InSetting);
}
