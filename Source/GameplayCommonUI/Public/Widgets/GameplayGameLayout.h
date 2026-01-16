// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplayActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "GameplayGameLayout.generated.h"

class UGameplayControllerDisconnectedScreen;

/**
 * @brief Root widget for managing the primary gameplay UI layout
 *
 * This widget is responsible for coordinating the standard HUD, handling the 
 * "Escape" action to open menus, and managing the "Controller Disconnected" 
 * logic for the local player.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DisableNativeTick))
class GAMEPLAYCOMMONUI_API UGameplayGameLayout : public UGameplayActivatableWidget
{
	GENERATED_BODY()
	
public:
	UGameplayGameLayout(const FObjectInitializer& ObjectInitializer);

	//~Begin UUserWidget Interface
	virtual void NativeOnInitialized() override;
	virtual void NativeDestruct() override;
	//~End of UUserWidget Interface
	
protected:
	/** Internal handler for the Escape/Back action */
	virtual void HandleEscapeAction();
	
	/** 
	 * @brief Callback for platform-level input device connection changes
	 * 
	 * Determines if the player has lost all controllers and triggers the 
	 * disconnected screen if necessary.
	 */
	virtual void HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId, FInputDeviceId InputDeviceId);
	
	/**
	 * @brief Callback for input device pairing changes
	 * 
	 * Checks if a reconnected controller is now paired with our player, 
	 * potentially dismissing the disconnect screen.
	 */
	virtual void HandleInputDevicePairingChanged(FInputDeviceId InputDeviceId, FPlatformUserId NewUserPlatformId, FPlatformUserId OldUserPlatformId);
	
	/** Queues a check of controller state for the next frame */
	virtual void NotifyControllerStateChangeForDisconnectScreen();
	
	/** Evaluates current controller connectivity and updates the disconnect screen visibility */
	virtual void ProcessControllerDevicesHavingChangedForDisconnectScreen();
	
	/** Returns true if the current platform requires a specialized disconnect screen */
	virtual bool ShouldPlatformDisplayControllerDisconnectScreen() const;
	
	/** 
	 * @brief Blueprint-native event to display the disconnect menu 
	 * 
	 * Pushes the ControllerDisconnectedScreen class to the default menu layer.
	 */
	UFUNCTION(BlueprintNativeEvent, Category = "Controller Disconnect Menu")
	void DisplayControllerDisconnectedMenu();
	
	/** Blueprint-native event to remove the disconnect menu */
	UFUNCTION(BlueprintNativeEvent, Category = "Controller Disconnect Menu")
	void HideControllerDisconnectedMenu();
	
protected:
	/** The widget class to spawn when the Escape/Pause action is triggered */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Screens")
	TSoftClassPtr<UCommonActivatableWidget> EscapeMenuClass;
	
	/** The specialized widget class for the controller disconnected state */
	UPROPERTY(EditDefaultsOnly, Category="Screens")
	TSubclassOf<UGameplayControllerDisconnectedScreen> ControllerDisconnectedScreen;
	
	/** 
	 * Platform tags required to enable controller disconnect screen logic. 
	 * If missing or not matching the platform, the logic remains inactive.
	 */
	UPROPERTY(EditDefaultsOnly, Category="Platform")
	FGameplayTagContainer PlatformRequiresControllerDisconnectScreen;
	
	/** Cached reference to the active disconnect screen instance */
	UPROPERTY(Transient)
	TObjectPtr<UCommonActivatableWidget> SpawnedControllerDisconnectedScreen;
	
	/** Handle for the deferred controller state processing task */
	FTSTicker::FDelegateHandle RequestProcessControllerStateHandle;
};

