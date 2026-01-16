// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonRotator.h"
#include "Input/NavigationReply.h"
#include "GameplayRotator.generated.h"

/**
 * @brief Extension of CommonRotator with support for custom navigation logic
 * 
 * Allows users to cycle through a list of options (typically text-based settings) 
 * using left/right inputs. Provides a simplified way to select options by their 
 * display text.
 */
UCLASS(Abstract, BlueprintType, meta=(DisableNativeTick))
class GAMEPLAYCOMMONUI_API UGameplayRotator : public UCommonRotator
{
	GENERATED_BODY()

public:
	UGameplayRotator(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UCommonRotator interface
	virtual bool Initialize() override;
	//~End of UCommonRotator interface

	//~Begin UUserWidget interface
	/** Internal delegate used to override standard UI navigation */
	FNavigationDelegate OnCustomNavigation;
	virtual FNavigationReply NativeOnNavigation(const FGeometry& InGeometry, const FNavigationEvent& InNavigationEvent, const FNavigationReply& InDefaultReply) override;
	//~End of UUserWidget interface
	
	/** Sets the current selection by matching the provided text against the available options */
	UFUNCTION(BlueprintCallable, Category="Option")
	void SetSelectedOptionByText(const FText& TextOption);

protected:
	/** Internal handler for custom navigation events */
	virtual TSharedPtr<SWidget> NativeOnCustomNavigation(EUINavigation UINavigation);
};

