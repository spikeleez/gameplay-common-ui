// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Misc/GameplaySubtitlesTypes.h"
#include "Engine/DataAsset.h"
#include "Fonts/SlateFontInfo.h"
#include "Styling/SlateBrush.h"
#include "GameplaySubtitleDisplayOptions.generated.h"

/**
 * @brief Data asset defining the standard visual parameters for subtitles
 * 
 * This asset allows artists and designers to define a palette of font sizes, 
 * colors, border styles, and background opacities that the subtitle system 
 * can draw from based on user settings.
 */
UCLASS(Blueprintable, BlueprintType, Const, meta=(DisplayName="Gameplay Subtitle Display Options"))
class GAMEPLAYCOMMONUI_API UGameplaySubtitleDisplayOptions : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UGameplaySubtitleDisplayOptions();
	
public:
	/** The base font information used for all subtitles */
	UPROPERTY(EditDefaultsOnly, Category = "Display")
	FSlateFontInfo SubtitleFont;
	
	/** Array of font sizes mapped to the EGameplaySubtitleDisplayTextSize enum */
	UPROPERTY(EditDefaultsOnly, Category = "Display")
	int32 SubtitleTextSizes[(int32)EGameplaySubtitleDisplayTextSize::MAX];
	
	/** Array of colors mapped to the EGameplaySubtitleDisplayTextColor enum */
	UPROPERTY(EditDefaultsOnly, Category = "Display")
	FLinearColor SubtitleTextColors[(int32)EGameplaySubtitleDisplayTextColor::MAX];
	
	/** Array of border thicknesses mapped to the EGameplaySubtitleDisplayTextBorder enum */
	UPROPERTY(EditDefaultsOnly, Category = "Display")
	float SubtitleBorderSizes[(int32)EGameplaySubtitleDisplayTextBorder::MAX];
	
	/** Array of background opacity values mapped to the EGameplaySubtitleDisplayBackgroundOpacity enum */
	UPROPERTY(EditDefaultsOnly, Category = "Display")
	float SubtitleBackgroundOpacities[(int32)EGameplaySubtitleDisplayBackgroundOpacity::MAX];
	
	/** The brush used to render the background box behind the subtitle text */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Display")
	FSlateBrush SubtitleBackgroundBrush;
};

