// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplaySetting.h"
#include "GameplaySettingAction.generated.h"

class ULocalPlayer;

/**
 * @brief Delegate for custom setting actions
 * @param Setting The setting that triggered the action
 * @param LocalPlayer The local player for which the action is being executed
 */
DECLARE_DELEGATE_TwoParams(UGameplayGameSettingCustomAction, UGameplaySetting* /** Setting */, ULocalPlayer* /** LocalPlayer */);

/**
 * @brief Represents a setting that performs an action when activated
 * 
 * UGameplaySettingAction is a special type of setting that doesn't store a value
 * but instead triggers an action when the user interacts with it. Actions can be:
 * 
 * - **Named Actions**: Using GameplayTags to identify specific actions (e.g., "Settings.ShowCredits")
 * - **Custom Actions**: Binding custom functions for specialized behavior
 * 
 * Common uses include:
 * - Opening supplementary screens (Credits, EULA, Privacy Policy)
 * - Triggering game-specific commands (Screenshot, Reset Controls)
 * - Executing confirmation dialogs
 * 
 * By default, actions don't mark settings as changed (dirty), but this
 * can be configured for actions that modify persistent state.
 * 
 * @see UGameplaySetting, FGameplayTag
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingAction : public UGameplaySetting
{
	GENERATED_BODY()

public:
	/** Event broadcasted when a named action is executed */
	DECLARE_EVENT_TwoParams(UGameplaySettingAction, FGameplayOnExecuteNamedAction, UGameplaySetting* /** Setting */, FGameplayTag /** ActionTag */);
	/** @brief Native event triggered when ExecuteAction is called with a NamedAction tag */
	FGameplayOnExecuteNamedAction OnExecuteNamedActionEvent;
	
public:
	UGameplaySettingAction(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	/**
	 * @brief Gets the text displayed on the action button
	 * @return The action button text
	 */
	FText GetActionText() const { return ActionText; }
	
	/**
	 * @brief Sets the text displayed on the action button
	 * @param Value The new action text
	 */
	void SetActionText(const FText& Value) { ActionText = Value; }
	
#if !UE_BUILD_SHIPPING
	/**
	 * @brief Sets the action text from a string (non-shipping only)
	 * @param Value The action text as a string
	 */
	void SetActionText(const FString& Value) { SetActionText(FText::FromString(Value)); }
#endif

	/**
	 * @brief Gets the gameplay tag for this named action
	 * @return The named action tag
	 */
	FGameplayTag GetNamedAction() const { return NamedAction; }
	
	/**
	 * @brief Sets the gameplay tag for this named action
	 * @param Value The named action tag to set
	 */
	void SetNamedAction(FGameplayTag Value) { NamedAction = Value; }

	/**
	 * @brief Checks if this setting has a custom action bound
	 * @return True if a custom action is bound
	 */
	bool HasCustomAction() const { return CustomAction.IsBound(); }
	
	/**
	 * @brief Sets a custom action delegate
	 * @param InAction The custom action delegate to bind
	 */
	void SetCustomAction(const UGameplayGameSettingCustomAction& InAction) { CustomAction = InAction; }
	
	/**
	 * @brief Sets a custom action from a lambda function
	 * @param InAction Lambda function that takes a ULocalPlayer*
	 */
	void SetCustomAction(TFunction<void(ULocalPlayer*)> InAction);

	/**
	 * @brief Configures whether this action dirties settings
	 * 
	 * By default, actions don't dirty the settings. If your action 
	 * modifies persistent state that should trigger save prompts, set this to true.
	 * 
	 * @param Value True if the action should mark settings as dirty
	 */
	void SetDoesActionDirtySettings(bool Value) { bDirtyAction = Value; }

	/**
	 * @brief Executes the action
	 * 
	 * Triggers either the named action (broadcasting the event) or the custom action if bound.
	 * Override this method in derived classes for specialized action behavior.
	 */
	virtual void ExecuteAction();

protected:
	/** The text displayed on the action button */
	FText ActionText;
	
	/** The gameplay tag identifying this named action */
	FGameplayTag NamedAction;
	
	/** Custom action delegate for specialized behavior */
	UGameplayGameSettingCustomAction CustomAction;
	
	/** Whether executing this action should mark settings as dirty */
	uint8 bDirtyAction:1;
	
protected:
	// ~Begin UGameplaySetting interface
	virtual void OnInitialized() override;
	// ~End of UGameplaySetting interface
};
