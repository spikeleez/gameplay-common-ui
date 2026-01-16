// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "UObject/Object.h"
#include "Widgets/GameplayPrimaryLayout.h"
#include "Misc/GameplayCommonTypes.h"
#include "Subsystems/GameplayCommonUISubsystem.h"
#include "GameplayCommonUIPolicy.generated.h"

class UGameplayPrimaryLayout;
class ULocalPlayer;

/**
 * @brief Policy class that manages the UI lifecycle and primary layout
 * 
 * The UI Policy is responsible for determining which primary layout class to use, 
 * creating it for local players, and managing its addition/removal from the viewport. 
 * It acts as the bridge between the GameplayCommonUISubsystem and the actual 
 * widget instances.
 */
UCLASS(BlueprintType, Blueprintable, Within=GameplayCommonUISubsystem)
class GAMEPLAYCOMMONUI_API UGameplayCommonUIPolicy : public UObject
{
	GENERATED_BODY()

public:
	UGameplayCommonUIPolicy(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UObject interface
	virtual UWorld* GetWorld() const override;
	//~End of UObject interface

	/**
	 * @brief Helper to get the UI policy cast to a specific class
	 * @tparam PolicyClass The class to cast to
	 * @param WorldContextObject Context to find the subsystem
	 * @return The policy instance or null
	 */
	template <typename PolicyClass = UGameplayCommonUIPolicy>
	static PolicyClass* GetUIPolicyAs(const UObject* WorldContextObject)
	{
		return Cast<PolicyClass>(GetUIPolicy(WorldContextObject));
	}
	
	/** Adds a primary layout to a local player's viewport */
	void AddPrimaryLayoutToViewport(ULocalPlayer* LocalPlayer, UGameplayPrimaryLayout* Layout);
	
	/** Removes a primary layout from a local player's viewport */
	void RemovePrimaryLayoutFromViewport(ULocalPlayer* LocalPlayer, UGameplayPrimaryLayout* Layout);
	
	/** Internal notification for when a player's pawn changes, allowing UI to react (e.g., HUD updates) */
	void NotifyPawnChanged(ULocalPlayer* LocalPlayer, APawn* NewPawn);

	/** Gets the active UI policy for the current world context */
	static const UGameplayCommonUIPolicy* GetUIPolicy(const UObject* WorldContextObject);
	
	/** Gets the subsystem that owns this policy */
	UGameplayCommonUISubsystem* GetOwningSubsystem() const;
	
	/** Gets the primary layout associated with a specific local player */
	UGameplayPrimaryLayout* GetPrimaryLayoutFromLocalPlayer(const ULocalPlayer* LocalPlayer) const;

	/** Gets the class of the primary layout to be created */
	UFUNCTION(BlueprintPure, Category = "Primary Layout")
	TSubclassOf<UGameplayPrimaryLayout> GetPrimaryLayoutClass();
	
	/** Gets the default primary layout (useful for single-player or shared UI) */
	UFUNCTION(BlueprintPure, Category = "Primary Layout")
	UGameplayPrimaryLayout* GetPrimaryLayout() const;

protected:
	/** Hook for when a primary layout is added to a viewport */
	virtual void OnPrimaryLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UGameplayPrimaryLayout* Layout);
	
	/** Hook for when a primary layout is removed from a viewport */
	virtual void OnPrimaryLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UGameplayPrimaryLayout* Layout);
	
	/** Hook for when a primary layout widget is actually being destroyed/released */
	virtual void OnPrimaryLayoutReleased(ULocalPlayer* LocalPlayer, UGameplayPrimaryLayout* Layout);

	/** Factory method to create a primary layout for a player */
	void CreatePrimaryLayout(ULocalPlayer* LocalPlayer);

private:
	/** The class of the widget used as the root layout */
	UPROPERTY(EditAnywhere, Category = "Primary Layout")
	TSubclassOf<UGameplayPrimaryLayout> PrimaryLayoutClass;

	/** Reference to the primary layout instance */
	UPROPERTY()
	TObjectPtr<UGameplayPrimaryLayout> PrimaryLayout;

	/** Tracks layouts across different viewports (for split-screen) */
	UPROPERTY(Transient)
	TArray<FGameplayPrimaryLayoutViewport> PrimaryViewportLayouts;

private:
	/** Internal callback for player connection */
	void NotifyPlayerAdded(ULocalPlayer* LocalPlayer);
	
	/** Internal callback for player disconnection */
	void NotifyPlayerRemoved(ULocalPlayer* LocalPlayer);
	
	/** Internal callback for subsystem destruction */
	void NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer);

	friend class UGameplayCommonUISubsystem;
	friend class UGameplayCommonLocalPlayerSubsystem;
};

