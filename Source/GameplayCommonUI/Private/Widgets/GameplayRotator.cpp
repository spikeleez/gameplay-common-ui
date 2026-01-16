// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayRotator.h"
#include "CommonTextBlock.h"

UGameplayRotator::UGameplayRotator(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

bool UGameplayRotator::Initialize()
{
	if (Super::Initialize())
	{
		OnCustomNavigation.BindUObject(this, &UGameplayRotator::NativeOnCustomNavigation);
		return true;
	}

	return false;
}

void UGameplayRotator::SetSelectedOptionByText(const FText& TextOption)
{
	const int32 FoundTextIndex = TextLabels.IndexOfByPredicate([TextOption](const FText& TextItem)->bool
	{
		return TextItem.EqualTo(TextOption);
	});

	if (FoundTextIndex != INDEX_NONE)
	{
		SetSelectedItem(FoundTextIndex);
		return;
	}

	MyText->SetText(TextOption);
}

FNavigationReply UGameplayRotator::NativeOnNavigation(const FGeometry& InGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply)
{
	switch (InNavigationEvent.GetNavigationType())
	{
	case EUINavigation::Left:
	case EUINavigation::Right:
		return FNavigationReply::Custom(OnCustomNavigation);
	default:
		return InDefaultReply;
	}
}

TSharedPtr<SWidget> UGameplayRotator::NativeOnCustomNavigation(EUINavigation UINavigation)
{
	const int32 Count = TextLabels.Num();

	const int32 CurrentIndex = GetSelectedIndex();
	const int32 PreviousIndex = GetSelectedIndex() - 1;
	const int32 NextIndex = GetSelectedIndex() + 1;

	if (UINavigation == EUINavigation::Left)
	{
		if (CurrentIndex > 0)
		{
			ShiftTextLeftInternal(true);
		}
	}
	else if (UINavigation == EUINavigation::Right)
	{
		if (CurrentIndex < Count - 1)
		{
			ShiftTextRightInternal(true);
		}
	}

	return nullptr;
}


