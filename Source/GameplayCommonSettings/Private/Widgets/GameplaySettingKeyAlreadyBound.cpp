// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplaySettingKeyAlreadyBound.h"
#include "CommonTextBlock.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingKeyAlreadyBound)

void UGameplaySettingKeyAlreadyBound::SetWarningText(const FText& InText)
{
	if (Text_Warning)
	{
		Text_Warning->SetText(InText);
	}
}

void UGameplaySettingKeyAlreadyBound::SetCancelText(const FText& InText)
{
	if (Text_Cancel)
	{
		Text_Cancel->SetText(InText);
	}
}
