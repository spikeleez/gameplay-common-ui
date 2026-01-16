// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CommonActivatableWidget.h"
#include "CommonUserWidget.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "Misc/GameplayCommonUILibrary.h"
#include "Misc/GameplayCommonTypes.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "Engine/AssetManager.h"
#include "GameplayPrimaryLayout.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogGameplayPrimaryLayout, Log, All);

/**
 * @brief The root UI layout for a single player
 *
 * This widget class acts as the container for all UI layers (HUD, Menus, Modals) 
 * for a specific player. It manages the registration of these layers and 
 * provides methods to push/pop activatable widgets onto specific stacks.
 */
UCLASS(Abstract, BlueprintType, meta = (DisableNativeTick))
class GAMEPLAYCOMMONUI_API UGameplayPrimaryLayout : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UGameplayPrimaryLayout(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	//~Begin UUserWidget interface
	virtual void NativeOnInitialized() override;
	//~End of UUserWidget interface
		
	/** Returns the primary layout for the first local player */
	static UGameplayPrimaryLayout* GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject);
	
	/** Returns the primary layout for a specific player controller */
	static UGameplayPrimaryLayout* GetPrimaryGameLayout(const APlayerController* PlayerController);
	
	/** Returns the primary layout for a specific local player */
	static UGameplayPrimaryLayout* GetPrimaryGameLayout(const ULocalPlayer* LocalPlayer);
	
	/** Returns the map of all registered layer containers by their tags */
	const TMap<FGameplayTag, UCommonActivatableWidgetContainerBase*>& GetRegisteredLayers() const { return RegisteredLayers; }

	/** 
	 * Sets whether this layout is dormant. 
	 * A dormant layout is usually collapsed and only responds to persistent global actions.
	 */
	void SetIsDormant(bool Dormant);
	
	/** Checks if the layout is currently dormant */
	bool IsDormant() const { return bIsDormant; }

	/** 
	 * @brief Asynchronously loads and pushes a widget to a specific layer
	 * @param LayerTag The tag of the layer to push to
	 * @param bSuspendInputUntilComplete Whether to block player input during the load
	 * @param ActivatableWidgetClass Soft class pointer to the widget to load
	 * @return A handle to the streaming operation
	 */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerTag, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass)
	{
		return PushWidgetToLayerStackAsync<ActivatableWidgetT>(LayerTag, bSuspendInputUntilComplete, ActivatableWidgetClass, [](EGameplayWidgetLayerAsyncState, ActivatableWidgetT*) {});
	}

	/** 
	 * @brief Asynchronously loads and pushes a widget to a specific layer with state callbacks
	 * @param LayerTag The tag of the layer to push to
	 * @param bSuspendInputUntilComplete Whether to block player input during the load
	 * @param ActivatableWidgetClass Soft class pointer to the widget to load
	 * @param StateFunc Callback function triggered at different stages of the async lifecycle
	 * @return A handle to the streaming operation
	 */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	TSharedPtr<FStreamableHandle> PushWidgetToLayerStackAsync(FGameplayTag LayerTag, bool bSuspendInputUntilComplete, TSoftClassPtr<UCommonActivatableWidget> ActivatableWidgetClass, TFunction<void(EGameplayWidgetLayerAsyncState, ActivatableWidgetT*)> StateFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		static FName NAME_PushingWidgetToLayer("PushingWidgetToLayer");
		const FName SuspendInputToken = bSuspendInputUntilComplete ? UGameplayCommonUILibrary::SuspendInputForPlayer(GetOwningPlayer(), NAME_PushingWidgetToLayer) : NAME_None;

		FStreamableManager& StreamableManager = UAssetManager::Get().GetStreamableManager();
		TSharedPtr<FStreamableHandle> StreamingHandle = StreamableManager.RequestAsyncLoad(ActivatableWidgetClass.ToSoftObjectPath(), FStreamableDelegate::CreateWeakLambda(this,
		[this, LayerTag, ActivatableWidgetClass, StateFunc, SuspendInputToken]()
		{
			UGameplayCommonUILibrary::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);

			ActivatableWidgetT* Widget = PushWidgetToLayerStack<ActivatableWidgetT>(LayerTag, ActivatableWidgetClass.Get(), [StateFunc](ActivatableWidgetT& WidgetToInit)
			{
				StateFunc(EGameplayWidgetLayerAsyncState::Initialize, &WidgetToInit);
			});

			StateFunc(EGameplayWidgetLayerAsyncState::AfterPush, Widget);
			Widget->SetFocus();
		}));

		// Set up a cancel delegate so that we can resume input if this handler is canceled.
		StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this, [this, StateFunc, SuspendInputToken]()
		{
			UGameplayCommonUILibrary::ResumeInputForPlayer(GetOwningPlayer(), SuspendInputToken);
			StateFunc(EGameplayWidgetLayerAsyncState::Canceled, nullptr);
		}));

		return StreamingHandle;
	}

	/** Synchronously pushes a widget to a layer stack */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerTag, UClass* ActivatableWidgetClass)
	{
		return PushWidgetToLayerStack<ActivatableWidgetT>(LayerTag, ActivatableWidgetClass, [](ActivatableWidgetT&) {});
	}

	/** Synchronously pushes and initializes a widget to a layer stack */
	template <typename ActivatableWidgetT = UCommonActivatableWidget>
	ActivatableWidgetT* PushWidgetToLayerStack(FGameplayTag LayerTag, UClass* ActivatableWidgetClass, TFunctionRef<void(ActivatableWidgetT&)> InitInstanceFunc)
	{
		static_assert(TIsDerivedFrom<ActivatableWidgetT, UCommonActivatableWidget>::IsDerived, "Only CommonActivatableWidgets can be used here");

		if (UCommonActivatableWidgetContainerBase* Layer = GetLayerStackContainer(LayerTag))
		{
			UE_LOG(LogGameplayPrimaryLayout, Display, TEXT("Pushing Widget [%s] to Layer [%s]"), *GetNameSafe(ActivatableWidgetClass), *LayerTag.ToString());
			
			return Layer->AddWidget<ActivatableWidgetT>(ActivatableWidgetClass, InitInstanceFunc);
		}
		
		UE_LOG(LogGameplayPrimaryLayout, Warning, TEXT("Failed to push widget [%s]: Layer [%s] not found!"), *GetNameSafe(ActivatableWidgetClass), *LayerTag.ToString());
		return nullptr;
	}

	/** Returns the container widget for a given layer tag */
	UFUNCTION(BlueprintPure, Category="Stack")
	UCommonActivatableWidgetContainerBase* GetWidgetStackLayer(const FGameplayTag& LayerTag) const;
	
	/** Searches through all layers for a specific widget instance and removes it */
	UFUNCTION(BlueprintCallable, Category = "Stack")
	void FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget);

	/** Removes all widgets from a specific layer */
	UFUNCTION(BlueprintCallable, Category = "Stack")
	void FindAndRemoveWidgetsFromLayer(FGameplayTag LayerTag);

	/** Removes all widgets from every registered layer */
	UFUNCTION(BlueprintCallable, Category = "Stack")
	void ClearAllLayerStacks();

	/** Gets the container for a specific layer tag */
	UFUNCTION(BlueprintPure, Category = "Stack")
	UCommonActivatableWidgetContainerBase* GetLayerStackContainer(FGameplayTag LayerTag);
	
	/** Helper to retrieve the registration name of a layer container (for debugging) */
	UFUNCTION(BlueprintPure, Category = "Stack")
	FString GetLayerNameFromContainer(const UCommonActivatableWidgetContainerBase* Container) const;

protected:
	/** Registers a layer container widget to a specific tag */
	UFUNCTION(BlueprintCallable, Category="Stack")
	void RegisterLayerStack(UPARAM(meta=(GameplayTagFilter="GameplayCommonUI.Layer")) FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* Stack);

	/** Called when the dormancy state changes */
	virtual void OnIsDormantChanged();
	
	/** Called when a layer container begins or ends a transition */
	virtual void OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning);
	
private:
	/** Internal dormancy flag */
	bool bIsDormant = false;

	/** History of input tokens used to suspend input during async operations */
	TArray<FName> SuspendInputTokens;
	
	/** Persistent map of layer tags to container widgets */
	UPROPERTY(Transient, meta=(GameplayTagFilter="GameplayCommonUI.Layer"))
	TMap<FGameplayTag, UCommonActivatableWidgetContainerBase*> RegisteredLayers;
};

