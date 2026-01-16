// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Misc/GameplayCommonTypes.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "Engine/TimerHandle.h"
#include "GameplayCommonLocalPlayerSubsystem.generated.h"

class UGameplayCommonUIPolicy;
class UGameplayConfirmationDescriptor;
class APlayerState;

/** @brief Native delegate for the result of a confirmation dialog */
DECLARE_DELEGATE_OneParam(FGameplayConfirmationDialogResultSignature, EGameplayConfirmationResult /* Result */)

/** @brief Broadcast when the local player's pawn is set or changed */
DECLARE_MULTICAST_DELEGATE_OneParam(FGameplayLocalPlayerPawnSetSignature, APawn* /*NewPawn*/);

/** @brief Broadcast when the local player's state is set or identified */
DECLARE_MULTICAST_DELEGATE_OneParam(FGameplayLocalPlayerStateSetSignature, APlayerState* /*NewPlayerState*/);

/**
 * @brief Subsystem managing UI-related state for a specific local player
 * 
 * This subsystem handles player-specific UI logic, such as showing confirmation 
 * or error dialogs, and tracking changes to the player's pawn and player state 
 * for UI updates.
 */
UCLASS(meta=(DisplayName="Gameplay Common Local Player Subsystem"))
class GAMEPLAYCOMMONUI_API UGameplayCommonLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	//~Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End of USubsystem interface
	
	//~Begin ULocalPlayerSubsystem interface
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;
	//~End of ULocalPlayerSubsystem interface
	
	/**
	 * @brief Displays a standard confirmation dialog
	 * @param Descriptor Visual configuration for the dialog
	 * @param ResultCallback Callback to execute when the dialog is closed
	 */
	virtual void ShowConfirmation(UGameplayConfirmationDescriptor* Descriptor, FGameplayConfirmationDialogResultSignature ResultCallback = FGameplayConfirmationDialogResultSignature());
	
	/**
	 * @brief Displays an error-specific confirmation dialog
	 * @param Descriptor Visual configuration for the dialog
	 * @param ResultCallback Callback to execute when the dialog is closed
	 */
	virtual void ShowError(UGameplayConfirmationDescriptor* Descriptor, FGameplayConfirmationDialogResultSignature ResultCallback = FGameplayConfirmationDialogResultSignature());
	
	/** Native event for when the pawn changes */
	FGameplayLocalPlayerPawnSetSignature OnLocalPlayerPawnSet;
	
	/** Native event for when the player state becomes valid/changes */
	FGameplayLocalPlayerStateSetSignature OnLocalPlayerStateSet;
	
protected:
	/** Internal handler for pawn change notifications */
	virtual void HandlePawnChanged(APawn* NewPawn);	
	
private:
	/** Polls or checks for player state validity if it's not immediately available */
	void CheckPlayerStateValidity();
	
private:
	/** Handle for the player state validity polling timer */
	FTimerHandle TimerHandle_CheckPlayerState;
	
	/** Cached reference to the player controller */
	TWeakObjectPtr<APlayerController> WeakPlayerController;
};

