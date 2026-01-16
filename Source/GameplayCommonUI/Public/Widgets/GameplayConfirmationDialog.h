// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplayActivatableWidget.h"
#include "Subsystems/GameplayCommonLocalPlayerSubsystem.h"
#include "Misc/GameplayCommonTypes.h"
#include "UObject/Object.h"
#include "GameplayConfirmationDialog.generated.h"

class UDynamicEntryBox;
class UCommonRichTextBlock;

/**
 * @brief Data structure defining the content and layout of a confirmation dialog
 * 
 * Includes the title, descriptive message, and an array of actions (buttons) 
 * to be displayed.
 */
UCLASS(BlueprintType, Blueprintable)
class GAMEPLAYCOMMONUI_API UGameplayConfirmationDescriptor : public UObject
{
	GENERATED_BODY()

public:
	/** The text displayed at the top of the dialog */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Descriptor")
	FText DialogTitle;

	/** The detailed information or prompt message shown in the body */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Descriptor")
	FText DialogMessage;

	/** List of buttons/actions to present to the player */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Descriptor")
	TArray<FGameplayConfirmationDialogAction> DialogButtons;
	
public:
	UGameplayConfirmationDescriptor(const FObjectInitializer& ObjectInitializer);

	/** Static helper to create an "OK" only descriptor */
	static UGameplayConfirmationDescriptor* CreateConfirmationOk(const FText& InDialogTitle, const FText& InDialogMessage);
	
	/** Static helper to create an "OK / Cancel" descriptor */
	static UGameplayConfirmationDescriptor* CreateConfirmationOkCancel(const FText& InDialogTitle, const FText& InDialogMessage);
	
	/** Static helper to create a "Yes" only descriptor */
	static UGameplayConfirmationDescriptor* CreateConfirmationYes(const FText& InDialogTitle, const FText& InDialogMessage);
	
	/** Static helper to create a "Yes / No" descriptor */
	static UGameplayConfirmationDescriptor* CreateConfirmationYesNo(const FText& InDialogTitle, const FText& InDialogMessage);
	
	/** Static helper to create a "Yes / No / Cancel" descriptor */
	static UGameplayConfirmationDescriptor* CreateConfirmationYesNoCancel(const FText& InDialogTitle, const FText& InDialogMessage);
};

/**
 * @brief Standard UI widget for modal confirmation and error dialogs
 * 
 * Works in tandem with UGameplayConfirmationDescriptor to dynamically build 
 * buttons and display messages. Broadcasts the user's choice via a callback 
 * when a button is clicked or the dialog is dismissed.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DisableNativeTick))
class GAMEPLAYCOMMONUI_API UGameplayConfirmationDialog : public UGameplayActivatableWidget
{
	GENERATED_BODY()

public:
	UGameplayConfirmationDialog(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UCommonActivatableWidget interface
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	//~End of UCommonActivatableWidget interface
	
	/** Initializes the dialog with data and a completion callback */
	virtual void SetupDialog(UGameplayConfirmationDescriptor* Descriptor, FGameplayConfirmationDialogResultSignature ResultCallback);
	
	/** Forces the dialog to close without a specific button click (broadcasting 'Killed') */
	virtual void KillDialog();

protected:
	/** Internal handler for button click events */
	virtual void NativeOnConfirmationDialogButtonClicked(EGameplayConfirmationResult Result);
	
	/** Blueprint hook for reacting to button clicks */
	UFUNCTION(BlueprintNativeEvent, Category="Confirmation Dialog", meta=(DisplayName = "On Confirmation Dialog Button Clicked"))
	void BP_OnConfirmationDialogButtonClicked(EGameplayConfirmationResult Result);

private:
	/** Title text widget */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UCommonRichTextBlock> CommonRichText_DialogTitle;

	/** Message body text widget */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UCommonRichTextBlock> CommonRichText_DialogMessage;

	/** Dynamic container for the action buttons */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, AllowPrivateAccess = true))
	TObjectPtr<UDynamicEntryBox> EntryBox_DialogButtons;

	/** Callback to execute when the dialog is resolved */
	FGameplayConfirmationDialogResultSignature OnResultCallback;
};

