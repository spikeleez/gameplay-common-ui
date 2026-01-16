// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CommonButtonBase.h"
#include "GameplayButtonBase.generated.h"

class UCommonBorder;
class UCommonLazyImage;
class UCommonTextBlock;

/**
 * @brief Base class for buttons within the Gameplay Common UI system
 * 
 * Extends CommonButtonBase with integrated support for text labels, icons 
 * (via lazy loading), and optional description text used for accessibility 
 * or tooltips.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONUI_API UGameplayButtonBase : public UCommonButtonBase
{
	GENERATED_BODY()

public:
	UGameplayButtonBase(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UUserWidget interface
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	//~End of UUserWidget interface

	//~Begin UCommonButtonBase interface
	virtual void NativeOnHovered() override;
	virtual void NativeOnUnhovered() override;
	virtual void OnInputMethodChanged(ECommonInputType CurrentInputType) override;
	virtual void UpdateInputActionWidget() override;
	virtual void NativeOnCurrentTextStyleChanged() override;
	//~End of UCommonButtonBase interface

	/** Refreshes the internal text widget based on current ButtonText and settings */
	virtual void RefreshButtonText();
	
	/** Sets and refreshes the main localized text shown on the button */
	UFUNCTION(BlueprintCallable, Category="Text")
	void SetButtonText(FText NewText);

	/** Returns the currently set main localized text for the button */
	UFUNCTION(BlueprintPure, Category="Text")
	FText GetButtonText() const;

	/** Manually sets the brush for the button's visual representation */
	UFUNCTION(BlueprintCallable, Category="Style")
	void SetButtonBrush(const FSlateBrush& NewBrush);
	
	/** Asynchronously loads and sets the button brush from a lazy-loadable object */
	UFUNCTION(BlueprintCallable, Category="Style")
	void SetButtonBrushFromLazyObject(TSoftObjectPtr<UObject> LazyObject);

	/** Returns the current visual brush of the button */
	UFUNCTION(BlueprintPure, Category="Style")
	FSlateBrush GetButtonBrush() const;
	
protected:
	/** Blueprint event to handle custom styling logic updates */
	UFUNCTION(BlueprintImplementableEvent, Category="Style", meta=(DisplayName = "Receive Update Button Style"))
	void ReceiveUpdateButtonStyle();
	
	/** Blueprint event to handle custom text update logic */
	UFUNCTION(BlueprintImplementableEvent, Category = "Text", meta=(DisplayName = "Receive Update Button Text"))
	void ReceiveUpdateButtonText(const FText& InText);

protected:
	/** Optional border widget defining the button background/frame */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidgetOptional, AllowPrivateAccess = true))
	TObjectPtr<UCommonBorder> Border_Button;
	
	/** Optional text widget for displaying the button label */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidgetOptional, AllowPrivateAccess = true))
	TObjectPtr<UCommonTextBlock> Text_Button;

	/** Optional lazy-loaded image widget for an icon or illustration */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidgetOptional, AllowPrivateAccess = true))
	TObjectPtr<UCommonLazyImage> Image_Button;
	
	/** If true, the button will attempt to display input prompt text (e.g., "[X] Accept") */
	UPROPERTY(EditAnywhere, Category="Text")
	uint8 bShowInputDisplayTextIfPossible : 1;

	/** The primary localized label text for this button */
	UPROPERTY(EditAnywhere, Category="Text")
	FText ButtonText;
	
	/** If true, forces the displayed button text to be uppercase regardless of input source */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Text")
	uint8 bButtonTextUppercase : 1;

	/** Secondary descriptive text, often broadcast to subsystems for tooltip display */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Text")
	FText ButtonDescriptionText;
};

