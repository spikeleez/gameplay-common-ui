// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "CommonUserWidget.h"
#include "Blueprint/UserWidgetPool.h"
#include "GameplaySettingDetailView.generated.h"

struct FStreamableHandle;
class UCommonRichTextBlock;
class UCommonTextBlock;
class UGameplaySettingVisualData;
class UGameplaySettingDetailExtension;
class UGameplaySetting;
class UVerticalBox;

/**
 * @brief Widget that displays detailed information and extended controls for a setting
 * 
 * The detail view typically appears when a setting is hovered or selected. It shows:
 * - Setting Name
 * - Description (Rich Text)
 * - Dynamic details (e.g. current value vs base value)
 * - Warnings or error messages
 * - Disabled reasons
 * - Any registered Detail Extensions (specialized sub-widgets)
 * 
 * It uses a widget pool to efficiently manage extension widgets.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingDetailView : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UGameplaySettingDetailView(const FObjectInitializer& ObjectInitializer);

	// ~Begin UVisual interface
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// ~End of UVisual interface
	
	/**
	 * @brief Populates the detail view with information from the provided setting
	 * @param InSetting The setting to inspect
	 */
	void FillSettingDetails(UGameplaySetting* InSetting);

protected:
	/** Data Asset mapping settings to their extension widgets */
	UPROPERTY(EditAnywhere, Category="Visual")
	TObjectPtr<UGameplaySettingVisualData> VisualData;

	/** Pool for reusing detail extension widgets */
	UPROPERTY(Transient)
	FUserWidgetPool ExtensionWidgetPool;

	/** The setting currently being displayed */
	UPROPERTY(Transient)
	TObjectPtr<UGameplaySetting> CurrentSetting;

	/** Handle for async loading of visual assets */
	TSharedPtr<FStreamableHandle> StreamingHandle;
	
protected:
	// ~Begin UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeOnInitialized() override;
	// ~End of UUserWidget interface

	/**
	 * @brief Creates and adds a detail extension widget to the view
	 * @param InSetting The setting context
	 * @param ExtensionClass The class of extension to instantiate
	 */
	void CreateDetailsExtension(UGameplaySetting* InSetting, TSubclassOf<UGameplaySettingDetailExtension> ExtensionClass);

private:
	/** Label for the setting name */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidgetOptional, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonTextBlock> Text_SettingName;

	/** Rich text block for the setting's main description */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidgetOptional, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonRichTextBlock> RichText_Description;

	/** Rich text block for dynamic, per-frame information about the setting */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidgetOptional, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonRichTextBlock> RichText_DynamicDetails;

	/** Rich text block for showing warnings (e.g., "Requires Restart") */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidgetOptional, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonRichTextBlock> RichText_WarningDetails;

	/** Rich text block for explaining why a setting is currently disabled */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidgetOptional, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonRichTextBlock> RichText_DisabledDetails;

	/** Container for hosting detail extension widgets */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidgetOptional, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UVerticalBox> Box_DetailsExtension;
};