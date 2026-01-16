// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Components/PanelWidget.h"
#include "Widgets/Layout/SGridPanel.h"
#include "Widgets/SCompoundWidget.h"
#include "GameplayResponsivePanel.generated.h"

class SGameplayResponsivePanel;
class UGameplayResponsivePanelSlot;
class FArrangedChildren;
class SWidget;
struct FGeometry;

/**
 * @brief Slate implementation of a panel that can wrap its content based on available space
 * 
 * SGameplayResponsivePanel acts as a wrapper around an SGridPanel, dynamically 
 * shifting child widgets between rows and columns depending on the screen size 
 * and configuration.
 */
class SGameplayResponsivePanel : public SCompoundWidget
{
public:
	typedef SGridPanel::FSlot FSlot;

public:
	SLATE_BEGIN_ARGS(SGameplayResponsivePanel)
	{
		_Visibility = EVisibility::SelfHitTestInvisible;
	}

	SLATE_END_ARGS()

public:
	SGameplayResponsivePanel();

	/**
	 * @brief Construct this widget
	 * @param InArgs The declaration data for this widget
	 */
	void Construct(const FArguments& InArgs);

	/**
	 * @brief Adds a content slot to the inner grid.
	 * @return The added slot.
	 */
	FSlot& AddSlot();

	/**
	 * @brief Removes a particular content slot.
	 * @param SlotWidget The widget in the slot to remove.
	 * @return The index of the removed slot
	 */
	int32 RemoveSlot(const TSharedRef<SWidget>& SlotWidget);

	/**
	 * @brief Removes all slots from the panel.
	 */
	void ClearChildren();

	/** @brief Sets whether the panel is allowed to wrap children vertically */
	void EnableVerticalStacking(const bool bCanVerticallyWrap);

protected:
	// ~Begin SWidget interface.
	virtual bool CustomPrepass(float LayoutScaleMultiplier) override;
	virtual FVector2D ComputeDesiredSize(float) const override;
	virtual float GetRelativeLayoutScale(int32 ChildIndex, float LayoutScaleMultiplier) const override;
	virtual void OnArrangeChildren(const FGeometry& AllottedGeometry, FArrangedChildren& ArrangedChildren) const override;
	// ~End SWidget interface.

	/** @brief Logic to determine if layout should wrap */
	bool ShouldWrap() const;

	/** @brief Updates internal measurement for screen responsiveness */
	void RefreshResponsiveness();
	
	/** @brief Rebuilds the grid layout based on current wrapping state */
	void RefreshLayout();

protected:
	/** Internal grid managing child layout */
	TSharedRef<SGridPanel> InnerGrid;

	/** Direct references to slots within the inner grid */
	TArray<SGridPanel::FSlot*> InnerSlots;

	/** Current detected physical screen size for scaling logic */
	FVector2D PhysicalScreenSize = FVector2D(0, 0);

	/** Current layout scale factor */
	float Scale = 1;

	/** If true, widgets can stack vertically when horizontal space is low */
	uint8 bCanWrapVertically : 1;
};

/**
 * @brief A container widget that allows child widgets to flow horizontally and wrap vertically
 *
 * UGameplayResponsivePanel is ideal for settings screens where some categories might 
 * need to stack on smaller screens or mobile devices while staying horizontal 
 * on desktop/widescreen displays.
 */
UCLASS()
class UGameplayResponsivePanel : public UPanelWidget
{
	GENERATED_BODY()

public:
	UGameplayResponsivePanel(const FObjectInitializer& ObjectInitializer);

	// ~Begin UWidget interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
#if WITH_EDITOR
	virtual const FText GetPaletteCategory() override;
#endif
	// ~End of UWidget interface
	
	/**
	 * @brief Adds a child widget to this responsive panel
	 * @param Content The widget to add
	 * @return The slot created for the added child
	 */
	UFUNCTION(BlueprintCallable, Category="Widget")
	UGameplayResponsivePanelSlot* AddChildToGameResponsivePanel(UWidget* Content);

public:
	/** @brief Whether children are allowed to stack vertically when they run out of horizontal space */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Behavior")
	bool bCanStackVertically = true;

protected:
	// ~Begin UPanelWidget interface
	virtual UClass* GetSlotClass() const override;
	virtual void OnSlotAdded(UPanelSlot* InSlot) override;
	virtual void OnSlotRemoved(UPanelSlot* InSlot) override;
	// ~End of UPanelWidget interface

	/** The underlying Slate implementation */
	TSharedPtr<SGameplayResponsivePanel> MyGameResponsivePanel;

protected:
	// ~Begin UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	// ~End of UWidget interface
};