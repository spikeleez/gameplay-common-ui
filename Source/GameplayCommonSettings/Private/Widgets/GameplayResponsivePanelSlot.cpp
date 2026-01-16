// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayResponsivePanelSlot.h"
#include "Components/Widget.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayResponsivePanelSlot)

UGameplayResponsivePanelSlot::UGameplayResponsivePanelSlot(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	Slot = nullptr;
}

void UGameplayResponsivePanelSlot::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	Slot = nullptr;
}

void UGameplayResponsivePanelSlot::BuildSlot(const TSharedRef<SGameplayResponsivePanel>& GameResponsivePanel)
{
	Slot = &GameResponsivePanel->AddSlot()
	[
		Content == nullptr ? SNullWidget::NullWidget : Content->TakeWidget()
	];
}

void UGameplayResponsivePanelSlot::SynchronizeProperties()
{
	
}
