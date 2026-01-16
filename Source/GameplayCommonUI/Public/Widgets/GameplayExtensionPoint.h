// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Components/DynamicEntryBoxBase.h"
#include "Subsystems/GameplayCommonExtensionSubsystem.h"
#include "Widgets/SOverlay.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SBorder.h"
#include "GameplayExtensionPoint.generated.h"

/**
 * @brief Widget that acts as an anchor for the UI Extension system
 * 
 * When placed in a widget tree, this component registers itself as an extension point 
 * with the GameplayCommonExtensionSubsystem using a specific tag. When extensions 
 * are registered to that tag, this widget will automatically create and manage 
 * the corresponding child widgets.
 */
UCLASS()
class GAMEPLAYCOMMONUI_API UGameplayExtensionPoint : public UDynamicEntryBoxBase
{
	GENERATED_BODY()
	
public:
	/** Delegate to determine which widget class to spawn for a given data item */
	DECLARE_DYNAMIC_DELEGATE_RetVal_OneParam(TSubclassOf<UUserWidget>, FOnGetWidgetClassForData, UObject*, DataItem);
	
	/** Delegate to perform custom initialization on a spawned extension widget */
	DECLARE_DYNAMIC_DELEGATE_TwoParams(FOnConfigureWidgetForData, UUserWidget*, Widget, UObject*, DataItem);
	
public:
	UGameplayExtensionPoint(const FObjectInitializer& ObjectInitializer);
	
	//~Begin UWidget interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const override;
#endif
	//~End of UWidget interface
	
protected:
	/** The tag that uniquely identifies this extension point in the subsystem */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	FGameplayTag ExtensionPointTag;

	/** How strictly the extension tags must match this point's tag */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	EGameplayUIExtensionPointMatch ExtensionPointTagMatch = EGameplayUIExtensionPointMatch::ExactMatch;

	/** If provided, only extensions with data matching these classes will be processed */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension")
	TArray<TObjectPtr<UClass>> DataClasses;

	/** Blueprint event to resolve a widget class from a data payload */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension", meta=(IsBindableEvent="True"))
	FOnGetWidgetClassForData GetWidgetClassForData;

	/** Blueprint event to configure a widget once it has been created for a data payload */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI Extension", meta=(IsBindableEvent="True"))
	FOnConfigureWidgetForData ConfigureWidgetForData;

	/** Handles to the registered points in the extension subsystem */
	TArray<FGameplayUIExtensionPointHandle> ExtensionPointHandles;

	/** Mapping of extension handles to the actual widget instances created by this point */
	UPROPERTY(Transient)
	TMap<FGameplayUIExtensionHandle, TObjectPtr<UUserWidget>> ExtensionMapping;
	
private:
	/** Clears all current extensions and unregisters from the subsystem */
	void ResetExtensionPoint();
	
	/** Initiates registration with the global extension subsystem */
	void RegisterExtensionPoint();
	
	/** Specific registration logic for when a player state becomes available */
	void RegisterExtensionPointForPlayerState(APlayerState* PlayerState);
	
	/** Callback from the subsystem when an extension matching our criteria is added or removed */
	void OnAddOrRemoveExtension(EGameplayUIExtensionAction Action, const FGameplayUIExtensionRequest& Request);
};

