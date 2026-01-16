// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Kismet/BlueprintAsyncActionBase.h"
#include "Misc/GameplayCommonTypes.h"
#include "GameplayTagContainer.h"
#include "Engine/LocalPlayer.h"
#include "GameplayShowConfirmationDialogAsync.generated.h"

class UGameplayConfirmationDescriptor;

/** @brief Delegate fired when the user selects an option in the confirmation dialog */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameplayAsyncConfirmationDialogResultSignature, EGameplayConfirmationResult, Result);

/**
 * @brief Async action to display a confirmation dialog
 *
 * This class provides several static methods to quickly trigger different 
 * standard confirmation dialogs (Yes/No, Ok/Cancel, etc.) from Blueprints 
 * and wait for the user's choice.
 */
UCLASS()
class GAMEPLAYCOMMONUI_API UGameplayShowConfirmationDialogAsync : public UBlueprintAsyncActionBase
{
	GENERATED_BODY()

public:
	/** Event triggered with the user's response once the dialog is closed */
	UPROPERTY(BlueprintAssignable)
	FGameplayAsyncConfirmationDialogResultSignature DialogResult;
	
public:
	UGameplayShowConfirmationDialogAsync(const FObjectInitializer& ObjectInitializer);
	
	//~Begin UBlueprintAsyncActionBase interface
	virtual void Activate() override;
	//~End of UBlueprintAsyncActionBase interface

	/**
	 * @brief Shows a simple dialog with only a "Yes" button
	 * @param WorldContext The object identifying the world context
	 * @param Title The localized title of the dialog
	 * @param Message The localized message content
	 * @param bShowError If true, the dialog may use an error-specific visual style
	 * @return The async action object
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GameplayTasks", meta=(WorldContext="WorldContext", BlueprintInternalUseOnly="true", DisplayName="Show Confirmation Yes"))
	static UGameplayShowConfirmationDialogAsync* ShowConfirmationYes(UObject* WorldContext, FText Title, FText Message, bool bShowError);

	/**
	 * @brief Shows a dialog with "Yes" and "No" buttons
	 * @param WorldContext The object identifying the world context
	 * @param Title The localized title of the dialog
	 * @param Message The localized message content
	 * @param bShowError If true, the dialog may use an error-specific visual style
	 * @return The async action object
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GameplayTasks", meta=(WorldContext="WorldContext", BlueprintInternalUseOnly="true", DisplayName="Show Confirmation Yes/No"))
	static UGameplayShowConfirmationDialogAsync* ShowConfirmationYesNo(UObject* WorldContext, FText Title, FText Message, bool bShowError);
	
	/**
	 * @brief Shows a dialog with "Yes", "No", and "Cancel" buttons
	 * @param WorldContext The object identifying the world context
	 * @param Title The localized title of the dialog
	 * @param Message The localized message content
	 * @param bShowError If true, the dialog may use an error-specific visual style
	 * @return The async action object
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GameplayTasks", meta=(WorldContext="WorldContext", BlueprintInternalUseOnly="true", DisplayName="Show Confirmation Yes/No/Cancel"))
	static UGameplayShowConfirmationDialogAsync* ShowConfirmationYesNoCancel(UObject* WorldContext, FText Title, FText Message, bool bShowError);

	/**
	 * @brief Shows a simple dialog with only an "Ok" button
	 * @param WorldContext The object identifying the world context
	 * @param Title The localized title of the dialog
	 * @param Message The localized message content
	 * @param bShowError If true, the dialog may use an error-specific visual style
	 * @return The async action object
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GameplayTasks", meta=(WorldContext="WorldContext", BlueprintInternalUseOnly="true", DisplayName="Show Confirmation Ok"))
	static UGameplayShowConfirmationDialogAsync* ShowConfirmationOk(UObject* WorldContext, FText Title, FText Message, bool bShowError);
	
	/**
	 * @brief Shows a dialog with "Ok" and "Cancel" buttons
	 * @param WorldContext The object identifying the world context
	 * @param Title The localized title of the dialog
	 * @param Message The localized message content
	 * @param bShowError If true, the dialog may use an error-specific visual style
	 * @return The async action object
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GameplayTasks", meta=(WorldContext="WorldContext", BlueprintInternalUseOnly="true", DisplayName="Show Confirmation Ok/Cancel"))
	static UGameplayShowConfirmationDialogAsync* ShowConfirmationOkCancel(UObject* WorldContext, FText Title, FText Message, bool bShowError);

	/**
	 * @brief Shows a custom dialog identified by a tag
	 * @param WorldContext The object identifying the world context
	 * @param DialogTag A tag that maps to a specific dialog configuration
	 * @param bShowError If true, the dialog may use an error-specific visual style
	 * @return The async action object
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GameplayTasks", meta=(WorldContext="WorldContext", BlueprintInternalUseOnly="true", DisplayName="Show Confirmation Custom"))
	static UGameplayShowConfirmationDialogAsync* ShowConfirmationCustom(UObject* WorldContext, FGameplayTag DialogTag, bool bShowError);

private:
	/** Internal handler for the result delegate from the confirmation subsystem */
	void HandleConfirmationResult(EGameplayConfirmationResult Result);

private:
	/** Context for world access */
	UPROPERTY(Transient)
	TObjectPtr<UObject> WorldContextObject;

	/** The player for whom the dialog is intended */
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> TargetLocalPlayer;

	/** The descriptor containing all the visual configuration for the dialog */
	UPROPERTY(Transient)
	TObjectPtr<UGameplayConfirmationDescriptor> Descriptor;

	/** Whether this is categorized as an error dialog */
	UPROPERTY(Transient)
	bool bIsErrorDialog;
};

