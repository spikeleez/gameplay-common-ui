// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Engine/DeveloperSettings.h"
#include "GameplayTagContainer.h"
#include "GameplayCommonUISettings.generated.h"

class UGameplayConfirmationDialog;
class UCommonActivatableWidget;
class UGameplayCommonUIPolicy;
class UGameplayConfirmationDescriptor;

/**
 * @brief Global developer settings for the Gameplay Common UI system
 * 
 * This class stores project-wide configurations that define which UI policy class to use, 
 * mappings for tags to activatable widgets, and default classes for confirmation dialogs. 
 * These settings are accessible via the Project Settings menu in the Unreal Editor.
 */
UCLASS(Config=Game, DefaultConfig, DisplayName="Gameplay Common: UI Settings")
class GAMEPLAYCOMMONUI_API UGameplayCommonUISettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UGameplayCommonUISettings(const FObjectInitializer& ObjectInitializer);
	
public:
	/** The class used to manage UI lifecycle and layouts */
	UPROPERTY(Config, EditAnywhere, Category="Common", meta=(DisplayName = "UI Policy Class"))
	TSoftClassPtr<UGameplayCommonUIPolicy> GameplayUIPolicyClass;

	/** A mapping of tags to activatable widget classes, allowing for decoupled widget loading by tag */
	UPROPERTY(Config, EditAnywhere, Category="Common", meta=(ForceInlineRow, DisplayName = "Registered Activatable Widgets"))
	TMap<FGameplayTag, TSoftClassPtr<UCommonActivatableWidget>> RegisteredActivatableWidgets;

	/** The default widget class used for standard confirmation dialogs */
	UPROPERTY(Config, EditAnywhere, Category="Confirmation Dialog", meta=(DisplayName = "Confirmation Dialog Class"))
	TSoftClassPtr<UGameplayConfirmationDialog> ConfirmationDialogClass;

	/** The default widget class used for error/critical confirmation dialogs */
	UPROPERTY(Config, EditAnywhere, Category="Confirmation Dialog", meta=(DisplayName = "Error Dialog Class"))
	TSoftClassPtr<UGameplayConfirmationDialog> ErrorDialogClass;
	
	/** A mapping of tags to custom dialog descriptor assets, allowing for pre-configured complex dialogs */
	UPROPERTY(Config, EditAnywhere, Category="Confirmation Dialog", meta = (ForceInlineRow, DisplayName = "Registered Dialog Descriptors"))
	TMap<FGameplayTag, TSoftClassPtr<UGameplayConfirmationDescriptor>> RegisteredDialogDescriptors;
};

