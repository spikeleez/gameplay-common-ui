// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayResponsivePanel.h"
#include "Widgets/GameplayResponsivePanelSlot.h"
#include "Widgets/SViewport.h"
#include "Framework/Application/SlateApplication.h"
#include "Android/AndroidPlatformApplicationMisc.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplayResponsivePanel)

#define LOCTEXT_NAMESPACE "GameplayGameSetting"

SGameplayResponsivePanel::SGameplayResponsivePanel()
	: InnerGrid(SNew(SGridPanel))
{
	SetCanTick(false);
	bCanSupportFocus = false;
	bHasCustomPrepass = true;
	bHasRelativeLayoutScale = true;
	bCanWrapVertically = true;
}

void SGameplayResponsivePanel::Construct(const FArguments& InArgs)
{
	ChildSlot
	[
		InnerGrid
	];
}

SGameplayResponsivePanel::FSlot& SGameplayResponsivePanel::AddSlot()
{
	SGridPanel::FSlot* Slot;
	InnerGrid->AddSlot(InnerGrid->GetChildren()->Num(), 0)
		.Expose(Slot);
	InnerSlots.Add(Slot);

	RefreshLayout();

	return *Slot;
}

int32 SGameplayResponsivePanel::RemoveSlot(const TSharedRef<SWidget>& SlotWidget)
{
	for (int32 SlotIdx = 0; SlotIdx < InnerSlots.Num(); ++SlotIdx)
	{
		if (SlotWidget == InnerSlots[SlotIdx]->GetWidget())
		{
			InnerSlots.RemoveAt(SlotIdx);
			break;
		}
	}

	return InnerGrid->RemoveSlot(SlotWidget);
}

void SGameplayResponsivePanel::ClearChildren()
{
	InnerGrid->ClearChildren();
}

void SGameplayResponsivePanel::EnableVerticalStacking(const bool bCanVerticallyWrap)
{
	bCanWrapVertically = bCanVerticallyWrap;
}

bool SGameplayResponsivePanel::CustomPrepass(float LayoutScaleMultiplier)
{
	RefreshResponsiveness();
	return true;
}

void SGameplayResponsivePanel::OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const
{
	ArrangedChildren.AddWidget(AllottedGeometry.MakeChild(
		ChildSlot.GetWidget(),
		FVector2D(0, 0),
		AllottedGeometry.GetLocalSize() / Scale,
		Scale
	));
}

FVector2D SGameplayResponsivePanel::ComputeDesiredSize(float InLayoutScale) const
{
	return SCompoundWidget::ComputeDesiredSize(InLayoutScale) * Scale;
}

float SGameplayResponsivePanel::GetRelativeLayoutScale(int32 ChildIndex, float LayoutScaleMultiplier) const
{
	return Scale;
}

bool SGameplayResponsivePanel::ShouldWrap() const
{
	if (PhysicalScreenSize.IsZero() || !bCanWrapVertically)
	{
		return false;
	}

	return (PhysicalScreenSize.X < 7);
}

void SGameplayResponsivePanel::RefreshResponsiveness()
{
	PhysicalScreenSize = FVector2D(0, 0);

	const TSharedPtr<SViewport> GameViewport = FSlateApplication::Get().GetGameViewport();
	if (GameViewport.IsValid())
	{
		const TSharedPtr<ISlateViewport> ViewportInterface = GameViewport->GetViewportInterface().Pin();
		if (ViewportInterface.IsValid())
		{
			const FIntPoint ViewportSize = ViewportInterface->GetSize();

			int32 ScreenDensity = 0;
			FPlatformApplicationMisc::GetPhysicalScreenDensity(ScreenDensity);
			
			if (ScreenDensity != 0)
			{
				PhysicalScreenSize = ViewportSize / (float)ScreenDensity;
			}
		}
	}

	const bool bShouldWrap = ShouldWrap();
	const float NewScale = bShouldWrap ? 1.5f : 1.0f;
	if (!FMath::IsNearlyEqual(NewScale, Scale))
	{
		Scale = NewScale;
		RefreshLayout();
		Invalidate(EInvalidateWidgetReason::Prepass);
	}
}

void SGameplayResponsivePanel::RefreshLayout()
{
	const bool bShouldWrap = ShouldWrap();

	InnerGrid->ClearFill();

	for (int32 SlotIdx = 0; SlotIdx < InnerSlots.Num(); ++SlotIdx)
	{
		InnerSlots[SlotIdx]->SetColumn(bShouldWrap ? 0 : SlotIdx);
		InnerSlots[SlotIdx]->SetRow(bShouldWrap ? SlotIdx : 0);

		if (!bShouldWrap)
		{
			InnerGrid->SetColumnFill(SlotIdx, 1.0f);
		}
	}

	if (bShouldWrap)
	{
		InnerGrid->SetColumnFill(0, 1.0f);
	}
}

UGameplayResponsivePanel::UGameplayResponsivePanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bIsVariable = false;
	SetVisibilityInternal(ESlateVisibility::SelfHitTestInvisible);
}

void UGameplayResponsivePanel::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	MyGameResponsivePanel.Reset();
}

UClass* UGameplayResponsivePanel::GetSlotClass() const
{
	return UGameplayResponsivePanelSlot::StaticClass();
}

void UGameplayResponsivePanel::OnSlotAdded(UPanelSlot* InSlot)
{
	// Add the child to the live canvas if it already exists
	if ( MyGameResponsivePanel.IsValid() )
	{
		CastChecked<UGameplayResponsivePanelSlot>(InSlot)->BuildSlot(MyGameResponsivePanel.ToSharedRef());
	}
}

void UGameplayResponsivePanel::OnSlotRemoved(UPanelSlot* InSlot)
{
	// Remove the widget from the live slot if it exists.
	if ( MyGameResponsivePanel.IsValid() && InSlot->Content)
	{
		const TSharedPtr<SWidget> Widget = InSlot->Content->GetCachedWidget();
		if ( Widget.IsValid() )
		{
			MyGameResponsivePanel->RemoveSlot(Widget.ToSharedRef());
		}
	}
}

UGameplayResponsivePanelSlot* UGameplayResponsivePanel::AddChildToGameResponsivePanel(UWidget* Content)
{
	return Cast<UGameplayResponsivePanelSlot>( Super::AddChild(Content) );
}

TSharedRef<SWidget> UGameplayResponsivePanel::RebuildWidget()
{
	MyGameResponsivePanel = SNew(SGameplayResponsivePanel);

	MyGameResponsivePanel->EnableVerticalStacking(bCanStackVertically);

	for ( UPanelSlot* PanelSlot : Slots )
	{
		if ( UGameplayResponsivePanelSlot* TypedSlot = Cast<UGameplayResponsivePanelSlot>(PanelSlot) )
		{
			TypedSlot->Parent = this;
			TypedSlot->BuildSlot(MyGameResponsivePanel.ToSharedRef());
		}
	}

	return MyGameResponsivePanel.ToSharedRef();
}

#if WITH_EDITOR
const FText UGameplayResponsivePanel::GetPaletteCategory()
{
	return LOCTEXT("Panel", "Panel");
}
#endif

#undef LOCTEXT_NAMESPACE