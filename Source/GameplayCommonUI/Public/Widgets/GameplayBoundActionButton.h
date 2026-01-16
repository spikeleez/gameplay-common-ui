// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Input/CommonBoundActionButton.h"
#include "GameplayBoundActionButton.generated.h"

/**
 * @brief Button widget that automatically displays the icon/text for a bound UI action
 * 
 * This button is typically used in footers or legends to show the player which 
 * key or button is currently bound to a specific menu action (e.g., "Back", "Accept").
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONUI_API UGameplayBoundActionButton : public UCommonBoundActionButton
{
	GENERATED_BODY()
	
public:
	UGameplayBoundActionButton(const FObjectInitializer& ObjectInitializer);
	
	//~Begin UCommonBoundActionButton interface
	virtual void NativeOnCurrentTextStyleChanged() override;
	//~End of UCommonBoundActionButton interface
};

