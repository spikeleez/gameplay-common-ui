// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayTabButtonBase.h"
#include "CommonLazyImage.h"

void UGameplayTabButtonBase::SetTabLabelInfo_Implementation(const FGameplayTabDescriptor& TabLabelInfo)
{
	SetButtonText(TabLabelInfo.TabDisplayName);
	
	if (TabLabelInfo.TabIcon.HasUObject())
	{
		Image_Button->SetVisibility(ESlateVisibility::HitTestInvisible);
		SetButtonBrush(TabLabelInfo.TabIcon);
	}
	else
	{
		Image_Button->SetVisibility(ESlateVisibility::Collapsed);
	}
}
