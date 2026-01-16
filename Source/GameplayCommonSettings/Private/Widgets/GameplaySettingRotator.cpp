// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplaySettingRotator.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingRotator)

UGameplaySettingRotator::UGameplaySettingRotator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameplaySettingRotator::SetDefaultOption(int32 DefaultOptionIndex)
{
	BP_OnDefaultOptionSpecified(DefaultOptionIndex);
}
