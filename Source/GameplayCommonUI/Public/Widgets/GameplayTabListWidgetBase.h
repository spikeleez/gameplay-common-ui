// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CommonTabListWidgetBase.h"
#include "Misc/GameplayCommonTypes.h"
#include "GameplayTabListWidgetBase.generated.h"

class UGameplayButtonBase;

/**
 * @brief Interface for buttons that can display tab information
 */
UINTERFACE(BlueprintType)
class UGameplayTabButtonInterface : public UInterface
{
	GENERATED_BODY()
};

class IGameplayTabButtonInterface
{
	GENERATED_BODY()

public:
	/** Sets the displayed data for the tab button */
	UFUNCTION(BlueprintNativeEvent, Category = "Tab")
	void SetTabLabelInfo(const FGameplayTabDescriptor& TabDescriptor);
};

/**
 * @brief Extension of the common tab list with support for dynamic registration and visibility
 * 
 * This widget manages a list of tabs, usually corresponding to pages in a switcher. 
 * It supports pre-registered tabs defined in the editor and dynamic tabs added at 
 * runtime.
 */
UCLASS(Abstract, Blueprintable, BlueprintType, meta = (DisableNativeTick))
class GAMEPLAYCOMMONUI_API UGameplayTabListWidgetBase : public UCommonTabListWidgetBase
{
	GENERATED_BODY()

public:
	/** Delegate broadcast when a new tab's content widget has been instantiated */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnTabContentCreated, FName, TabId, UCommonUserWidget*, TabWidget);
	
	/** Native-only event broadcast when a new tab's content widget has been instantiated */
	DECLARE_EVENT_TwoParams(UGameplayTabListWidgetBase, FOnTabContentCreatedNative, FName /* TabId */, UCommonUserWidget* /* TabWidget */);

	/** Blueprint hook for tab creation events */
	UPROPERTY(BlueprintAssignable, Category = "TabList")
	FOnTabContentCreated OnTabContentCreated;
	
	/** Native hook for tab creation events */
	FOnTabContentCreatedNative OnTabContentCreatedNative;
	
public:
	UGameplayTabListWidgetBase(const FObjectInitializer& ObjectInitializer);
	
	/** Retrieves a descriptor from the pre-registered list in the editor */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TabList")
	bool GetPreregisteredTabDescriptor(const FName TabNameId, FGameplayTabDescriptor& OutTabInfo);
	
	/** Retrieves a currently active tab descriptor by ID */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TabList")
	bool GetTabDescriptor(const FName TabNameId, FGameplayTabDescriptor& OutTabInfo);

	/** Returns the complete array of editor-defined tab descriptors */
	const TArray<FGameplayTabDescriptor>& GetAllPreregisteredTabInfos() { return PreregisteredTabInfoArray; }

	/** Changes whether a tab is hidden from the UI (must be called before switcher link) */
	UFUNCTION(BlueprintCallable, Category = "TabList")
	void SetTabHiddenState(FName TabNameId, bool bHidden);

	/** Adds a new tab at runtime */
	UFUNCTION(BlueprintCallable, Category = "TabList")
	bool RegisterDynamicTab(const FGameplayTabDescriptor& TabDescriptor);

	/** Returns true if the first available tab is currently active */
	UFUNCTION(BlueprintCallable, Category = "TabList")
	bool IsFirstTabActive() const;

	/** Returns true if the last available tab is currently active */
	UFUNCTION(BlueprintCallable, Category = "TabList")
	bool IsLastTabActive() const;

	/** Checks if a specific tab is currently visible in the list */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TabList")
	bool IsTabVisible(FName TabId);

	/** Returns the total number of non-hidden tabs */
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "TabList")
	int32 GetVisibleTabCount();

protected:
	//~Begin UUserWidget interface
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	//~End of UUserWidget interface
	
	//~Begin UCommonTabListWidgetBase interface
	virtual void HandlePreLinkedSwitcherChanged() override;
	virtual void HandlePostLinkedSwitcherChanged() override;
	virtual void HandleTabCreation_Implementation(FName TabId, UCommonButtonBase* TabButton) override;
	//~End of UCommonTabListWidgetBase interface
	
	/** Internal method to populate the tab list from defined descriptors */
	void SetupTabs();

private:
	/** Editor-defined list of tabs for this widget */
	UPROPERTY(EditAnywhere, Category="TabList", meta=(TitleProperty="TabId"))
	TArray<FGameplayTabDescriptor> PreregisteredTabInfoArray;
	
	/** Internal buffer for tabs that are registered but whose buttons haven't spawned yet */
	UPROPERTY()
	TMap<FName, FGameplayTabDescriptor> PendingTabLabelInfoMap;
	
	/** Complete mapping of IDs to descriptors for all currently registered tabs */
	UPROPERTY()
	TMap<FName, FGameplayTabDescriptor> RegisteredTabDescriptors;
};
