// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Components/Widget.h"
#include "Misc/GameplaySubtitlesTypes.h"
#include "Styling/CoreStyle.h"
#include "Styling/SlateTypes.h"
#include "Styling/SlateWidgetStyleAsset.h"
#include "Styling/SlateWidgetStyleContainerBase.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Accessibility/SlateWidgetAccessibleTypes.h"
#include "Widgets/Layout/SBorder.h"
#include "GameplaySubtitleDisplay.generated.h"

class UGameplaySubtitleDisplayOptions;
class SRichTextBlock;
class FText;
struct FSlateBrush;

/**
 * @brief Slate implementation of the subtitle display widget
 */
class GAMEPLAYCOMMONUI_API SGameplaySubtitleDisplay : public SCompoundWidget
{
public:
	
	SLATE_BEGIN_ARGS(SGameplaySubtitleDisplay)
		: _TextStyle( &FCoreStyle::Get().GetWidgetStyle<FTextBlockStyle>("NormalText") )
		, _WrapTextAt(0.f)
		, _ManualSubtitles(false)
	{}

	/** The text style used for subtitle rendering */
	SLATE_STYLE_ARGUMENT( FTextBlockStyle, TextStyle )

	/** Width at which text should wrap; 0 or negative for no wrapping */
	SLATE_ATTRIBUTE( float, WrapTextAt )
	
	/** If true, the system won't automatically push subtitles here from the global subsystem */
	SLATE_ATTRIBUTE(bool, ManualSubtitles )
		
	SLATE_END_ARGS()

	virtual ~SGameplaySubtitleDisplay() override;
	
	/** Constructs the slate widget */
	void Construct( const FArguments& InArgs );

	/** Updates the text style at runtime */
	void SetTextStyle(const FTextBlockStyle& InTextStyle);

	/** Updates the background box brush at runtime */
	void SetBackgroundBrush(const FSlateBrush* InSlateBrush);
	
	/** Manually sets the current subtitle text to display */
	void SetCurrentSubtitleText(const FText& InSubtitleText);
	
	/** Configures the wrapping width */
	void SetWrapTextAt(const TAttribute<float>& InWrapTextAt);
	
	/** Returns true if there is currently active subtitle text */
	bool HasSubtitles() const;
	
private:
	/** Callback for when the global subtitle text changes */
	void HandleSubtitleChanged(const FText& SubtitleText);
	
private:
	/** Border widget for the background */
	TSharedPtr<SBorder> Background;
	
	/** Rich text block for displaying the content */
	TSharedPtr<SRichTextBlock> TextDisplay;
};

/**
 * @brief UWidget wrapper for displaying subtitles in UMG
 * 
 * Synchronizes with the GameplaySubtitlesSubsystem to automatically display 
 * subtitle text. Supports dynamic styling via FGameplaySubtitleFormat and 
 * UGameplaySubtitleDisplayOptions.
 */
UCLASS(BlueprintType, Blueprintable, meta=(DisableNativeTick))
class UGameplaySubtitleDisplay : public UWidget
{
	GENERATED_BODY()

public:
	UGameplaySubtitleDisplay(const FObjectInitializer& ObjectInitializer);
	
	//~Begin UWidget
	virtual void SynchronizeProperties() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	virtual TSharedRef<SWidget> RebuildWidget() override;
#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const override;
#endif
	//~End of UWidget
	
	/** Returns true if there is currently subtitle text being displayed */
	UFUNCTION(BlueprintCallable, Category="Subtitles", meta=(Tooltip = "True if there are subtitles currently.  False if the subtitle text is empty."))
	bool HasSubtitles() const;
	
public:
	/** Specific formatting settings (size, color, border) for this display */
	UPROPERTY(EditAnywhere, Category="Subtitle")
	FGameplaySubtitleFormat SubtitleFormat;
	
	/** Data asset containing the mapping of format enums to actual visual parameters */
	UPROPERTY(EditAnywhere, Category="Subtitle")
	TObjectPtr<UGameplaySubtitleDisplayOptions> SubtitleOptions;
	
	/** Maximum width before the text wraps to the next line */
	UPROPERTY(EditAnywhere, Category="Subtitle")
	float SubtitleWrapTextAt;
	
	/** If true, displays the SubtitlePreviewText in the editor even when no subtitles are active */
	UPROPERTY(EditAnywhere, Category="Subtitle|Preview")
	uint8 bSubtitlePreviewMode : 1;
	
	/** Text to show in the editor for layout purposes when preview mode is on */
	UPROPERTY(EditAnywhere, Category="Subtitle|Preview")
	FText SubtitlePreviewText;
	
protected:
	/** Internal handler for format property changes */
	virtual void HandleSubtitleDisplayFormatChanged(const FGameplaySubtitleFormat& NewSubtitleFormat);
	
private:
	/** Recalculates the GeneratedStyle and background brush based on current options */
	void RebuildStyle();
	
private:
	/** Derived text style for the Slate widget */
	UPROPERTY(Transient)
	FTextBlockStyle GeneratedStyle;
	
	/** Derived background brush for the Slate widget */
	UPROPERTY(Transient)
	FSlateBrush GeneratedBackgroundBorder;
	
	/** Internal Slate widget instance */
	TSharedPtr<SGameplaySubtitleDisplay> SubtitleWidget;
};

