// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Engine/CancellableAsyncAction.h"
#include "GameplayPushActivatableWidgetAsync.generated.h"

class UGameplayPrimaryLayout;
class APlayerController;
class UCommonActivatableWidget;
class UObject;

struct FFrame;
struct FStreamableHandle;

/** @brief Delegate for when a widget is about to be pushed or has been pushed */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameplayAsyncPushActivatableWidgetSignature, UCommonActivatableWidget*, UserWidget);

/**
 * @brief Async action to load and push a widget to a specific UI layer
 * 
 * This action handles the asynchronous loading of a widget (either by class 
 * or by tag) and then pushes it onto a designated layer within the 
 * UGameplayPrimaryLayout.
 */
UCLASS(BlueprintType)
class GAMEPLAYCOMMONUI_API UGameplayPushActivatableWidgetAsync : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/** Event fired after the widget is loaded but BEFORE it is pushed to the layer */
	UPROPERTY(BlueprintAssignable)
	FGameplayAsyncPushActivatableWidgetSignature BeforePush;

	/** Event fired AFTER the widget has been successfully pushed and activated */
	UPROPERTY(BlueprintAssignable)
	FGameplayAsyncPushActivatableWidgetSignature AfterPush;

public:
	UGameplayPushActivatableWidgetAsync(const FObjectInitializer& ObjectInitializer);
	
	//~Begin UCancellableAsyncAction interface
	virtual void Activate() override;
	virtual void Cancel() override;
	//~End of UCancellableAsyncAction interface

	/**
	 * @brief Asynchronously pushes a widget of a specific class to a layer
	 * 
	 * @param OwningPlayer The player that will own the widget
	 * @param WidgetClass Soft reference to the activatable widget class
	 * @param LayerTag The UI layer identifying tag where the widget should be pushed
	 * @param bSuspendInputUntilComplete If true, input is suspended while loading
	 * @return The async action object
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GameplayTasks", meta=(WorldContext="WorldContextObject", BlueprintInternalUseOnly="true", DisplayName="Push Activatable Widget for Class"))
	static UGameplayPushActivatableWidgetAsync* PushActivatableWidgetClassToLayer(APlayerController* OwningPlayer, UPARAM(meta = (AllowAbstract=false)) TSoftClassPtr<UCommonActivatableWidget> WidgetClass, UPARAM(meta=(GameplayTagFilter="GameplayCommonUI.Layer")) FGameplayTag LayerTag, bool bSuspendInputUntilComplete = true);

	/**
	 * @brief Asynchronously pushes a widget identified by a tag to a layer
	 * 
	 * @param OwningPlayer The player that will own the widget
	 * @param WidgetTag The tag mapping to a widget class (usually via UI policy/settings)
	 * @param LayerTag The UI layer identifying tag where the widget should be pushed
	 * @param bSuspendInputUntilComplete If true, input is suspended while loading
	 * @return The async action object
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GameplayTasks", meta=(WorldContext="WorldContextObject", BlueprintInternalUseOnly="true", DisplayName="Push Activatable Widget for Tag"))
	static UGameplayPushActivatableWidgetAsync* PushActivatableWidgetTagToLayer(APlayerController* OwningPlayer, UPARAM(meta = (Categories = "GameplayCommonUI.Widget")) FGameplayTag WidgetTag, UPARAM(meta=(GameplayTagFilter="GameplayCommonUI.Layer")) FGameplayTag LayerTag, bool bSuspendInputUntilComplete = true);
	
protected:
	/** Callback for when the primary layout becomes available or is already set */
	UFUNCTION()
	void OnPrimaryLayoutSet(UGameplayPrimaryLayout* PrimaryLayout);
	
private:
	/** The target UI layer tag */
	UPROPERTY(Transient)
	FGameplayTag MyLayerTag;
	
	/** The tag of the widget to push (if using tag-based lookup) */
	UPROPERTY(Transient)
	FGameplayTag MyWidgetTag;
	
	/** Whether to suspend input during load */
	UPROPERTY(Transient)
	bool bSuspendInputUntilComplete;
	
	/** Local player that will own the widget */
	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerController> OwningPlayerPtr;
	
	/** Soft class pointer of the widget to push */
	UPROPERTY(Transient)
	TSoftClassPtr<UCommonActivatableWidget> MyWidgetClass;
	
	/** Handle for the asynchronous resource load */
	TSharedPtr<FStreamableHandle> StreamingHandle;
};

