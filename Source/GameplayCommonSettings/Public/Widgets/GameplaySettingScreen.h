// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Framework/GameplaySettingRegistryChangeTracker.h"
#include "Widgets/GameplayActivatableWidget.h"
#include "GameplaySettingScreen.generated.h"

class UGameplaySettingPanel;
class UGameplaySetting;
class UGameplaySettingRegistry;
class UGameplaySettingCollection;
class UInputAction;

enum class EGameplaySettingChangeReason : uint8;

/**
 * @brief Top-level activatable widget for a full settings menu screen
 * 
 * UGameplaySettingScreen provides the high-level orchestration for a settings menu, including:
 * - Creating and managing the UGameplaySettingRegistry
 * - Tracking change state (dirty/applied) via FGameplaySettingRegistryChangeTracker
 * - Handling global actions like Apply, Cancel, and Reset
 * - Input action mapping for standard settings operations
 * - Managing the underlying UGameplaySettingPanel
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingScreen : public UGameplayActivatableWidget
{
	GENERATED_BODY()
	
public:
	/** @brief The class of registry to instantiate for this screen */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Registry")
	TSubclassOf<UGameplaySettingRegistry> RegistryClass;
	
public:
	UGameplaySettingScreen();
	
	// ~Begin UUserWidget interface
	virtual void NativeOnInitialized() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	// ~End of UUserWidget interface

	// ~Begin UCommonActivatableWidget interface
	virtual void NativeOnActivated() override;
	// ~End of UCommonActivatableWidget interface

	/**
	 * @brief Navigates the settings list to focus a specific setting
	 * @param SettingDevName Developer name of the setting to find
	 */
	UFUNCTION(BlueprintCallable, Category="Setting Screen")
	void NavigateToSetting(FName SettingDevName);
	
	/**
	 * @brief Navigates to a filtered view of multiple settings
	 * @param SettingDevNames List of developer names to include in the view
	 */
	UFUNCTION(BlueprintCallable, Category="Setting Screen")
	void NavigateToSettings(const TArray<FName>& SettingDevNames);

	/** @brief Called when the dirty state of the settings registry changes */
	UFUNCTION(BlueprintNativeEvent)
	void OnSettingsDirtyStateChanged(bool bSettingsDirty);

	/**
	 * @brief Attempts to pop the sub-page navigation stack
	 * @return True if a page was popped
	 */
	UFUNCTION(BlueprintCallable, Category="Setting Screen")
	bool AttemptToPopNavigation();

	/**
	 * @brief Retrieves a setting collection by name
	 * @param SettingDevName The name of the collection
	 * @param HasAnySettings [Out] True if the collection contains any settings
	 * @return The collection object if found
	 */
	UFUNCTION(BlueprintCallable, Category="Setting Screen")
	UGameplaySettingCollection* GetSettingCollection(FName SettingDevName, bool& HasAnySettings);

protected:
	/** Internal tracker for auditing which settings have been modified */
	FGameplaySettingRegistryChangeTracker ChangeTracker;

protected:
	/** @brief Overridable method to handle registry instantiation logic */
	virtual UGameplaySettingRegistry* CreateRegistry();
	
	/** @brief Helper to get the registry casted to a specific type */
	template <typename GameSettingRegistryT = UGameplaySettingRegistry>
	GameSettingRegistryT* GetRegistry() const { return Cast<GameSettingRegistryT>(const_cast<UGameplaySettingScreen*>(this)->GetOrCreateRegistry()); }

	/** @brief Reverts all unapplied changes in the registry and tracker */
	UFUNCTION(BlueprintCallable, Category="Setting Screen")
	virtual void CancelChanges();

	/** @brief Saves all pending changes in the registry to disk/application */
	UFUNCTION(BlueprintCallable, Category="Setting Screen")
	virtual void ApplyChanges();

	/** @brief Returns true if any setting has a pending modification */
	UFUNCTION(BlueprintCallable, Category="Setting Screen")
	bool HaveSettingsBeenChanged() const { return ChangeTracker.HaveSettingsBeenChanged(); }
	
	/** @brief Action handler for the Apply input */
	virtual void HandleApplyAction();

	/** @brief Action handler for the Cancel input */
	virtual void HandleCancelAction();

	/** @brief Action handler for the Return/Back input */
	virtual void HandleReturnAction();

	/** @brief Clears the internal dirty state tracking */
	void ClearDirtyState();
	
	/** @brief Internal callback for registry change notifications */
	void HandleSettingChanged(UGameplaySetting* Setting, EGameplaySettingChangeReason Reason);

private:
	/** Lazy-loads or returns the existing registry */
	UGameplaySettingRegistry* GetOrCreateRegistry();

private:	
	/** The panel widget containing the actual settings list and details */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta = (BindWidget, BlueprintProtected = true, AllowPrivateAccess = true))
	TObjectPtr<UGameplaySettingPanel> Settings_Panel;

	/** The active settings registry */
	UPROPERTY(Transient)
	mutable TObjectPtr<UGameplaySettingRegistry> Registry;

	/** Input action for Applying changes */
	UPROPERTY(EditAnywhere, Category = "Setting Screen")
	TObjectPtr<UInputAction> ApplyAction;
	FUIActionBindingHandle ApplyActionBindingHandle;

	/** Input action for Canceling changes */
	UPROPERTY(EditAnywhere, Category = "Setting Screen")
	TObjectPtr<UInputAction> CancelAction;
	FUIActionBindingHandle CancelActionBindingHandle;

	/** @brief Input action for Returning to previous page/screen */
	UPROPERTY(EditAnywhere, Category = "Setting Screen")
	TObjectPtr<UInputAction> ReturnAction;
	FUIActionBindingHandle ReturnActionBindingHandle;
};