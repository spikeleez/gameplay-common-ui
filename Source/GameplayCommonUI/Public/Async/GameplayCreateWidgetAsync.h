// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Engine/CancellableAsyncAction.h"
#include "GameplayCreateWidgetAsync.generated.h"

class APlayerController;
class UGameInstance;
class UUserWidget;
class UWorld;

struct FFrame;
struct FStreamableHandle;

/** @brief Delegate for when the async widget creation is complete */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FGameplayAsyncCreateWidgetSignature, UUserWidget*, UserWidget);

/**
 * @brief Async action to load and create a widget
 * 
 * This action handles the asynchronous loading of a widget class and its 
 * subsequent instantiation. It can optionally suspend player input while 
 * the loading is in progress to prevent interaction during transitions.
 */
UCLASS(BlueprintType)
class GAMEPLAYCOMMONUI_API UGameplayCreateWidgetAsync : public UCancellableAsyncAction
{
	GENERATED_BODY()

public:
	/** Event fired when the widget class has been loaded and the widget instance created */
	UPROPERTY(BlueprintAssignable)
	FGameplayAsyncCreateWidgetSignature OnComplete;
	
public:
	UGameplayCreateWidgetAsync(const FObjectInitializer& ObjectInitializer);
	
	//~Begin UCancellableAsyncAction interface
	virtual void Activate() override;
	virtual void Cancel() override;
	//~End of UCancellableAsyncAction interface

	/**
	 * @brief Asynchronously loads a widget class and creates an instance
	 * 
	 * @param WorldContextObject The context from which to access the world
	 * @param OwningPlayer The player controller that will own the new widget
	 * @param WidgetClass Soft reference to the widget class to load
	 * @param bSuspendInputUntilComplete If true, input will be suspended for the player until the widget is created
	 * @return The async action object
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category="GameplayTasks", meta=(WorldContext="WorldContextObject", BlueprintInternalUseOnly="true", DisplayName="Create Widget Async"))
	static UGameplayCreateWidgetAsync* CreateWidgetAsync(UObject* WorldContextObject, APlayerController* OwningPlayer, TSoftClassPtr<UUserWidget> WidgetClass, bool bSuspendInputUntilComplete = true);

protected:
	/** Internal callback for when the streamable handle finishes loading the class */
	void OnWidgetLoaded();
	
private:
	/** Token identifying the input suspension, used for restoration */
	UPROPERTY(Transient)
	FName SuspendInputToken;
	
	/** Whether input should be suspended during the load */
	UPROPERTY(Transient)
	bool bSuspendInputUntilComplete;
	
	/** Player that will own the widget */
	UPROPERTY(Transient)
	TWeakObjectPtr<APlayerController> OwningPlayer;
	
	/** World context */
	UPROPERTY(Transient)
	TWeakObjectPtr<UWorld> World;
	
	/** Game instance context */
	UPROPERTY(Transient)
	TWeakObjectPtr<UGameInstance> GameInstance;
	
	/** Soft class pointer to the widget we want to load */
	UPROPERTY(Transient)
	TSoftClassPtr<UUserWidget> UserWidgetSoftClass;
	
	/** Handle for the asynchronous resource load */
	TSharedPtr<FStreamableHandle> StreamingHandle;
};

