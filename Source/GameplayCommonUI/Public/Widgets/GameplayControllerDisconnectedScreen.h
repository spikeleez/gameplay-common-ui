// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "GenericPlatform/GenericPlatformApplicationMisc.h"
#include "GameplayControllerDisconnectedScreen.generated.h"

class UCommonButtonBase;
class UHorizontalBox;

/**
 * @brief UI screen displayed when player controllers are disconnected
 *
 * This widget prompts the user to reconnect their input device. On platforms 
 * with strict user pairing (like some consoles), it also provides functionality 
 * to switch the active platform user.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONUI_API UGameplayControllerDisconnectedScreen : public UCommonActivatableWidget
{
	GENERATED_BODY()
	
public:
	UGameplayControllerDisconnectedScreen(const FObjectInitializer& ObjectInitializer);
	
	//~Begin UUserWidget interface
	virtual void NativeOnActivated() override;
	//~End of UUserWidget interface
	
protected:
	/** Internal handler for when the 'Change User' button is clicked */
	virtual void HandleChangeUserClicked();
	
	/**
	 * @brief Callback for when the platform-level user selection process completes
	 * @param Params Results from the platform user selection
	 */
	virtual void HandleChangeUserCompleted(const FPlatformUserSelectionCompleteParams& Params);
	
	/**
	 * @brief Determines if the 'Change User' functionality should be visible
	 * @return True if the current platform and configuration support user switching here
	 */
	virtual bool ShouldDisplayChangeUserButton() const;
	
protected:
	/**
	 * Platform trait tags that determine if "Change User" should be allowed. 
	 * Typically checks for tags like "Platform.Trait.Input.HasStrictControllerPairing".
	 */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Platform")
	FGameplayTagContainer PlatformSupportsUserChangeTags;
	
	/** 
	 * Container for user switching UI elements 
	 * Will be hidden if ShouldDisplayChangeUserButton() returns false.
	 */
	UPROPERTY(BlueprintReadOnly, Category = "Designer", meta = (BindWidget))
	TObjectPtr<UHorizontalBox> HBox_SwitchUser;
	
	/** Button to initiate the platform-specific user switching flow */
	UPROPERTY(BlueprintReadOnly, Category = "Designer", meta = (BindWidget))
	TObjectPtr<UCommonButtonBase> Button_ChangeUser;
};

