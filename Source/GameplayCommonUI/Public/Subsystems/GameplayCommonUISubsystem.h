// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/HUD.h" // For AHUD definition
#include "Engine/Canvas.h" // For UCanvas definition
#include "GameplayCommonUISubsystem.generated.h"

class UGameplayButtonBase;
class UGameplayCommonUIPolicy;
class UGameplayPrimaryLayout;
class ULocalPlayer;

/** @brief Broadcast when the global primary layout is set or changed */
DECLARE_MULTICAST_DELEGATE_OneParam(FGameplayPrimaryLayoutSetSignature, UGameplayPrimaryLayout*);

/** @brief Broadcast when a button's description text changes, allowing for dynamic UI updates (e.g., tooltips or legends) */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FGameplayButtonDescriptionChangedSignature, UGameplayButtonBase*, Button, FText, NewDescriptionText);

/**
 * @brief Core GameInstance subsystem for managing the global UI lifecycle
 * 
 * This subsystem acts as the central coordinator for the UI system. It initializes 
 * the configured UI policy, manages player connection/disconnection events, 
 * and provides global accessibility to the active UI policy.
 */
UCLASS(meta=(DisplayName="Gameplay Common UI Subsystem"))
class GAMEPLAYCOMMONUI_API UGameplayCommonUISubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	/** Native event fired when the primary layout is established */
	FGameplayPrimaryLayoutSetSignature OnPrimaryLayoutSet;
	
	/** Blueprint-assignable event for when button descriptions change globally */
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FGameplayButtonDescriptionChangedSignature OnButtonDescriptionChanged;
	
public:
	UGameplayCommonUISubsystem() { }
	
	/** Static helper to retrieve the subsystem for a given world context */
	static UGameplayCommonUISubsystem* Get(const UObject* WorldContextObject);

	//~Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End of USubsystem interface

	/** Notification for when a new local player is added to the game session */
	virtual void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);
	
	/** Notification for when a local player is removed (e.g., logout or split-screen stop) */
	virtual void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);
	
	/** Notification for when a local player is being completely destroyed */
	virtual void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);
	
	/** Forces a broadcast of the button description change event */
	virtual void NotifyButtonDescriptionTextChanged(UGameplayButtonBase* Button, const FText& NewDescriptionText);
	
	/** Returns the currently active UI policy */
	UFUNCTION(BlueprintPure, Category="Policy")
	UGameplayCommonUIPolicy* GetUIPolicy() const { return CurrentPolicy; }

protected:
	/** Internal method to swap the current UI policy (used during initialization) */
	void SwitchToPolicy(UGameplayCommonUIPolicy* NewPolicy);
	
	/** Hook for rendering debug information to the screen via HUD */
	virtual void OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos);
	
private:
	/** The instance of the active UI policy */
	UPROPERTY(Transient)
	TObjectPtr<UGameplayCommonUIPolicy> CurrentPolicy;
};

