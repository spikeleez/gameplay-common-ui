// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "Misc/GameplayCommonTypes.h"
#include "GameplayActivatableWidget.generated.h"

class UInputAction;
struct FUIInputConfig;
struct FUIActionBindingHandle;

/** @brief Delegate for when a standard UI action is executed */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnInputActionExecutedSignature, FName, ActionName);

/** @brief Delegate for when an Enhanced Input action is executed */
DECLARE_DYNAMIC_DELEGATE_OneParam(FOnEnhancedInputActionExecutedSignature, UInputAction*, InputAction);

/**
 * @brief Base class for widgets that can be activated and deactivated within the UI stack
 * 
 * Provides enhanced support for input configuration (Menu vs Game vs Both) 
 * and automated binding/unbinding of UI actions based on the widget's lifecycle.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONUI_API UGameplayActivatableWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UGameplayActivatableWidget(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UUserWidget interface
	virtual void NativeDestruct() override;
	//~End of UUserWidget interface

	//~Begin UCommonActivatableWidget interface
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~End of UCommonActivatableWidget interface

#if WITH_EDITOR
	virtual void ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, IWidgetCompilerLog& CompileLog) const override;
#endif

	/**
	 * @brief Registers a standard UI action binding for this widget
	 * @param InputAction Row handle from the UI action data table
	 * @param Callback Function to call when the action is triggered
	 * @param BindingHandle [Out] Handle identifying the registration for later removal
	 */
	UFUNCTION(BlueprintCallable, Category="Bound Action")
	void RegisterBoundActionBinding(FDataTableRowHandle InputAction, const FOnInputActionExecutedSignature& Callback, FUIActionBindingHandle& BindingHandle);
	
	/**
	 * @brief Registers an Enhanced Input action binding for this widget
	 * @param InputAction The enhanced input action to listen for
	 * @param Callback Function to call when the action is triggered
	 * @param BindingHandle [Out] Handle identifying the registration for later removal
	 */
	UFUNCTION(BlueprintCallable, Category="Bound Action")
	void RegisterBoundEnhancedActionBinding(UInputAction* InputAction, const FOnEnhancedInputActionExecutedSignature& Callback, FUIActionBindingHandle& BindingHandle);

	/** @brief Manually unregisters a specific action binding */
	UFUNCTION(BlueprintCallable, Category="Bound Action")
	void UnregisterBoundActionBinding(FUIActionBindingHandle& BindingHandle);

	/** @brief Unregisters all currently active action bindings for this widget */
	UFUNCTION(BlueprintCallable, Category="Bound Action")
	void UnregisterAllBoundActionBindings();

protected:
	/** The input mode desired by this widget when activated */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input")
	EGameplayActivatableInputMode InputMode;

	/** How the mouse should be captured when in a Game-related input mode */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input", meta=(EditCondition = "InputMode != EGameplayActivatableInputMode::Menu", EditConditionHides))
	EMouseCaptureMode MouseCaptureMode;

	/** Whether to hide the mouse cursor while it's captured in the viewport */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Input", meta=(EditCondition = "InputMode != EGameplayActivatableInputMode::Menu", EditConditionHides))
	uint8 bHidesCursorDuringViewportCapture : 1;

private:
	/** Internal list of active action binding handles */
	TArray<FUIActionBindingHandle> BindingHandles;
};

