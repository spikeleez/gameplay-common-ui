// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayCommonUILibrary.generated.h"

class UGameplayPrimaryLayout;
enum class ECommonInputType : uint8;
template <typename T> class TSubclassOf;

class UUserWidget;
class ULocalPlayer;
class APlayerController;
class UCommonActivatableWidget;

/**
 * @brief General purpose utility library for Gameplay UI operations
 * 
 * Contains a variety of helpers for checking player input types, managing 
 * the UI stack (pushing/popping widgets), and handling input suspension.
 */
UCLASS()
class GAMEPLAYCOMMONUI_API UGameplayCommonUILibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UGameplayCommonUILibrary() { }

	/** Returns the current input type (Mouse/Keyboard, Gamepad, Touch) for the widget's owning player */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Gameplay UI Library", meta = (WorldContext = "WidgetContextObject"))
	static ECommonInputType GetOwningPlayerInputType(const UUserWidget* WidgetContextObject);

	/** Returns true if the widget's owning player is currently using touch input */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Gameplay UI Library", meta = (WorldContext = "WidgetContextObject"))
	static bool IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject);

	/** Returns true if the widget's owning player is currently using a gamepad */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Gameplay UI Library", meta = (WorldContext = "WidgetContextObject"))
	static bool IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject);

	/** Immediately removes a specific activatable widget from the UI stack */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Library")
	static void PopSingleWidget(UCommonActivatableWidget* ActivatableWidget);

	/** Removes all widgets from a specific UI layer for the given player */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Library")
	static void PopWidgetsFromLayer(const APlayerController* OwningPlayer, FGameplayTag LayerTag);

	/** Removes all widgets from the primary layout (clears the screen) for the given player */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Library")
	static void PopWidgetsFromLayout(APlayerController* OwningPlayer);

	/** Helper to safely cast a PlayerController to a LocalPlayer */
	UFUNCTION(BlueprintPure, BlueprintCosmetic, Category = "Gameplay UI Library")
	static ULocalPlayer* GetLocalPlayerFromController(const APlayerController* PlayerController);

	/**
	 * @brief Suspends game input for a specific player
	 * @param PlayerController The player to suspend input for
	 * @param SuspendReason A unique name identifying why input is suspended
	 * @return The token used for suspension
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Library")
	static FName SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason);
	
	/** Native helper to suspend input via LocalPlayer */
	static FName SuspendInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendReason);

	/**
	 * @brief Resumes game input for a specific player
	 * @param PlayerController The player to resume input for
	 * @param SuspendReason The identifying name used during suspension
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Library")
	static void ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendReason);
	
	/** Native helper to resume input via LocalPlayer */
	static void ResumeInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendReason);
	
	/**
	 * @brief Executes a callback when the primary layout for the given player is ready
	 * 
	 * If the layout is already initialized, the callback is executed immediately. 
	 * Otherwise, it will wait for the layout to be created.
	 */
	static void OnPrimaryLayoutReady(const APlayerController* OwningPlayer, TFunction<void(UGameplayPrimaryLayout*)> Callback);
	
	/** Native: Pushes a widget class to a layer synchronously */
	static UCommonActivatableWidget* PushActivatableWidgetForClass(const APlayerController* OwningPlayer, FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass);
	
	/** Native: Pushes a widget identified by a tag to a layer synchronously */
	static UCommonActivatableWidget* PushActivatableWidgetForTag(const APlayerController* OwningPlayer, FGameplayTag LayerTag, FGameplayTag WidgetTag);
	
	/** Native: Pushes a widget class asynchronously (streaming) */
	static void PushStreamedActivatableWidgetForClass(const APlayerController* OwningPlayer, FGameplayTag LayerTag, TSoftClassPtr<UCommonActivatableWidget> WidgetClass);
	
	/** Native: Pushes a widget identified by a tag asynchronously (streaming) */
	static void PushStreamedActivatableWidgetForTag(const APlayerController* OwningPlayer, FGameplayTag LayerTag, FGameplayTag WidgetTag);

private:
	/** Global counter for input suspensions across all players (used to generate tokens) */
	static int32 InputSuspensions;
};

