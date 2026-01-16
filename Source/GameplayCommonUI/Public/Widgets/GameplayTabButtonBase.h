// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplayButtonBase.h"
#include "GameplayTabListWidgetBase.h"
#include "GameplayTabButtonBase.generated.h"

/**
 * @brief Base class for buttons used within a UGameplayTabListWidgetBase
 * 
 * Implements IGameplayTabButtonInterface to automatically receive and 
 * display information from a FGameplayTabDescriptor (such as icons and 
 * localized names).
 */
UCLASS(Abstract, Blueprintable, meta = (DisableNativeTick))
class GAMEPLAYCOMMONUI_API UGameplayTabButtonBase : public UGameplayButtonBase, public IGameplayTabButtonInterface
{
	GENERATED_BODY()

public:
	//~Begin IGameplayTabButtonInterface interface
	/** Updates the button's visuals based on the provided tab descriptor */
	virtual void SetTabLabelInfo_Implementation(const FGameplayTabDescriptor& TabLabelInfo) override;
	//~End of IGameplayTabButtonInterface interface
};

