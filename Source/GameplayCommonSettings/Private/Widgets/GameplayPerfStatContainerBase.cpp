// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayPerfStatContainerBase.h"
#include "Widgets/GameplayPerfStatWidgetBase.h"
#include "Blueprint/WidgetTree.h"
#include "Framework/GameplaySettingsLocal.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayPerfStatContainerBase)

UGameplayPerfStatContainerBase::UGameplayPerfStatContainerBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
    
}

void UGameplayPerfStatContainerBase::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateVisibilityOfChildren();

	UGameplaySettingsLocal::Get()->OnPerfStatDisplayStateChanged().AddUObject(this, &UGameplayPerfStatContainerBase::UpdateVisibilityOfChildren);
}

void UGameplayPerfStatContainerBase::NativeDestruct()
{
	UGameplaySettingsLocal::Get()->OnPerfStatDisplayStateChanged().RemoveAll(this);

	Super::NativeDestruct();
}

void UGameplayPerfStatContainerBase::UpdateVisibilityOfChildren()
{
	const UGameplaySettingsLocal* UserSettings = UGameplaySettingsLocal::Get();
	if (!UserSettings) return;

	const bool bShowTextWidgets = (StatDisplayModeFilter == EGameplayPerformanceStatDisplayMode::TextOnly) || (StatDisplayModeFilter == EGameplayPerformanceStatDisplayMode::TextAndGraph);
	const bool bShowGraphWidgets = (StatDisplayModeFilter == EGameplayPerformanceStatDisplayMode::GraphOnly) || (StatDisplayModeFilter == EGameplayPerformanceStatDisplayMode::TextAndGraph);

	check(WidgetTree);
	WidgetTree->ForEachWidget([&](UWidget* Widget)
	{
		if (UGameplayPerfStatWidgetBase* TypedWidget = Cast<UGameplayPerfStatWidgetBase>(Widget))
		{
			const EGameplayPerformanceStatDisplayMode SettingMode = UserSettings->GetPerfStatDisplayState(TypedWidget->GetStatToDisplay());

			bool bShowWidget = false;
			switch (SettingMode)
			{
			case EGameplayPerformanceStatDisplayMode::Hidden:
				bShowWidget = false;
				break;
			case EGameplayPerformanceStatDisplayMode::TextOnly:
				bShowWidget = bShowTextWidgets;
				break;
			case EGameplayPerformanceStatDisplayMode::GraphOnly:
				bShowWidget = bShowGraphWidgets;
				break;
			case EGameplayPerformanceStatDisplayMode::TextAndGraph:
				bShowWidget = bShowTextWidgets || bShowGraphWidgets;
				break;
			}

			TypedWidget->SetVisibility(bShowWidget ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Collapsed);
		}
	});
}