// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplaySubtitleDisplay.h"
#include "SubtitleManager.h"
#include "Kismet/GameplayStatics.h"
#include "Misc/GameplaySubtitleDisplayOptions.h"
#include "Subsystems/GameplaySubtitlesSubsystem.h"
#include "Widgets/Text/SRichTextBlock.h"
#include "Engine/GameInstance.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif

#define LOCTEXT_NAMESPACE "GameplaySubtitleDisplay"

SGameplaySubtitleDisplay::~SGameplaySubtitleDisplay()
{
	FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText().RemoveAll(this);
}

void SGameplaySubtitleDisplay::Construct(const FArguments& InArgs)
{
	if (!InArgs._ManualSubtitles.Get())
	{
		FSubtitleManagerSetSubtitleText& OnSetSubtitleText = FSubtitleManager::GetSubtitleManager()->OnSetSubtitleText();
		OnSetSubtitleText.AddSP(this, &SGameplaySubtitleDisplay::HandleSubtitleChanged);
	}

	ChildSlot
	[
		SAssignNew(Background, SBorder)
		.Visibility(EVisibility::Collapsed)
		.Padding(FMargin(7.0, 5.0))
		[
			SAssignNew(TextDisplay, SRichTextBlock)
			.TextStyle(InArgs._TextStyle)
			.Justification(ETextJustify::Center)
			.WrapTextAt(InArgs._WrapTextAt)
		]
	];
}

void SGameplaySubtitleDisplay::SetTextStyle(const FTextBlockStyle& InTextStyle)
{
	TextDisplay->SetTextStyle(InTextStyle);
}

void SGameplaySubtitleDisplay::SetBackgroundBrush(const FSlateBrush* InSlateBrush)
{
	Background->SetBorderImage(InSlateBrush);
}

void SGameplaySubtitleDisplay::SetCurrentSubtitleText(const FText& InSubtitleText)
{
	Background->SetVisibility(InSubtitleText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible);
	TextDisplay->SetText(InSubtitleText);
}

bool SGameplaySubtitleDisplay::HasSubtitles() const
{
	return !TextDisplay->GetText().IsEmpty();
}

void SGameplaySubtitleDisplay::SetWrapTextAt(const TAttribute<float>& InWrapTextAt)
{
	TextDisplay->SetWrapTextAt(InWrapTextAt);
}

void SGameplaySubtitleDisplay::HandleSubtitleChanged(const FText& InSubtitleText)
{
	if (UGameplayStatics::AreSubtitlesEnabled())
	{
		Background->SetVisibility(InSubtitleText.IsEmpty() ? EVisibility::Collapsed : EVisibility::HitTestInvisible);
		TextDisplay->SetText(InSubtitleText);
	}
	else
	{
		Background->SetVisibility(EVisibility::Collapsed);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////

UGameplaySubtitleDisplay::UGameplaySubtitleDisplay(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SubtitleWrapTextAt(0)
{
	
}

#if WITH_EDITOR
void UGameplaySubtitleDisplay::ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);
	
	if (!SubtitleOptions)
	{
		CompileLog.Error(FText::Format(LOCTEXT("Error_UGameplaySubtitleDisplay_MissingOptions", "{0} has no subtitle Options asset specified."), FText::FromString(GetName())));
	}
}
#endif

void UGameplaySubtitleDisplay::SynchronizeProperties()
{
	Super::SynchronizeProperties();
	
	RebuildStyle();
	
	if (IsDesignTime() || bSubtitlePreviewMode)
	{
		SubtitleWidget->SetCurrentSubtitleText(SubtitlePreviewText);
	}
}

void UGameplaySubtitleDisplay::ReleaseSlateResources(bool bReleaseChildren)
{
	Super::ReleaseSlateResources(bReleaseChildren);

	SubtitleWidget.Reset();

	if (UGameplaySubtitlesSubsystem* SubtitleDisplay = UGameInstance::GetSubsystem<UGameplaySubtitlesSubsystem>(GetGameInstance()))
	{
		SubtitleDisplay->SubtitleFormatChangedEvent.RemoveAll(this);
	}
}

TSharedRef<SWidget> UGameplaySubtitleDisplay::RebuildWidget()
{
	if (UGameplaySubtitlesSubsystem* SubtitleDisplay = UGameInstance::GetSubsystem<UGameplaySubtitlesSubsystem>(GetGameInstance()))
	{
		SubtitleDisplay->SubtitleFormatChangedEvent.AddUObject(this, &ThisClass::HandleSubtitleDisplayFormatChanged);
		SubtitleFormat = SubtitleDisplay->GetSubtitleDisplayFormat();
	}

	SubtitleWidget = SNew(SGameplaySubtitleDisplay)
		.TextStyle(&GeneratedStyle)
		.WrapTextAt(SubtitleWrapTextAt)
		.ManualSubtitles(IsDesignTime() || bSubtitlePreviewMode);

	RebuildStyle();
	
	return SubtitleWidget.ToSharedRef();
}

bool UGameplaySubtitleDisplay::HasSubtitles() const
{
	return (SubtitleWidget.IsValid() && SubtitleWidget->HasSubtitles());
}

void UGameplaySubtitleDisplay::HandleSubtitleDisplayFormatChanged(const FGameplaySubtitleFormat& NewSubtitleFormat)
{
	if (SubtitleWidget.IsValid())
	{
		SubtitleFormat = NewSubtitleFormat;
		RebuildStyle();
	}
}

void UGameplaySubtitleDisplay::RebuildStyle()
{
	GeneratedStyle = FTextBlockStyle();

	if (SubtitleOptions)
	{
		GeneratedStyle.Font = SubtitleOptions->SubtitleFont;
		GeneratedStyle.Font.Size = SubtitleOptions->SubtitleTextSizes[(int32)SubtitleFormat.SubtitleTextSize];
		GeneratedStyle.ColorAndOpacity = SubtitleOptions->SubtitleTextColors[(int32)SubtitleFormat.SubtitleTextColor];

		switch (SubtitleFormat.SubtitleTextBorder)
		{
		case EGameplaySubtitleDisplayTextBorder::DropShadow:
			{
				const float ShadowSize = FMath::Max(1.0f, SubtitleOptions->SubtitleBorderSizes[(int32)EGameplaySubtitleDisplayTextBorder::DropShadow] * (float)SubtitleFormat.SubtitleTextSize / 2.0f);
				GeneratedStyle.SetShadowOffset(FVector2D(ShadowSize, ShadowSize));
				break;
			}
		case EGameplaySubtitleDisplayTextBorder::Outline:
			{
				const float OutlineSize = FMath::Max(1.0f, SubtitleOptions->SubtitleBorderSizes[(int32)EGameplaySubtitleDisplayTextBorder::Outline] * (float)SubtitleFormat.SubtitleTextSize);
				GeneratedStyle.Font.OutlineSettings.OutlineSize = OutlineSize;
				break;
			}
		case EGameplaySubtitleDisplayTextBorder::None:
		default:
			break;
		}

		FLinearColor CurrentBackgroundColor = SubtitleOptions->SubtitleBackgroundBrush.TintColor.GetSpecifiedColor();
		CurrentBackgroundColor.A = SubtitleOptions->SubtitleBackgroundOpacities[(int32)SubtitleFormat.SubtitleBackgroundOpacity];
		GeneratedBackgroundBorder = SubtitleOptions->SubtitleBackgroundBrush;
		GeneratedBackgroundBorder.TintColor = CurrentBackgroundColor;

		if (SubtitleWidget.IsValid())
		{
			SubtitleWidget->SetTextStyle(GeneratedStyle);
			SubtitleWidget->SetBackgroundBrush(&GeneratedBackgroundBorder);
		}
	}
}

#undef LOCTEXT_NAMESPACE
