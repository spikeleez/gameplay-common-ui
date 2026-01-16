// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplayResponsivePanel.h"
#include "Components/PanelSlot.h"
#include "GameplayResponsivePanelSlot.generated.h"

/**
 * @brief Slot for the UGameplayResponsivePanel, managing the layout data for a child widget
 */
UCLASS()
class UGameplayResponsivePanelSlot : public UPanelSlot
{
	GENERATED_BODY()

public:
	UGameplayResponsivePanelSlot(const FObjectInitializer& ObjectInitializer);
	
	/** @brief Assigns this slot to the specified Slate panel */
	void BuildSlot(const TSharedRef<SGameplayResponsivePanel>& GameResponsivePanel);

	// ~Begin UPanelSlot interface
	virtual void SynchronizeProperties() override;
	// ~End of UPanelSlot interface

	// ~Begin UWidget interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// ~End of UWidget interface

private:
	/** Pointer to the underlying Slate slot identifier in the responsive panel */
	SGameplayResponsivePanel::FSlot* Slot;
};