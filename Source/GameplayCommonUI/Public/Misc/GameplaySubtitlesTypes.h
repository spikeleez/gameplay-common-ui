// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplaySubtitlesTypes.generated.h"

/** @brief Options for subtitle text scaling */
UENUM(BlueprintType)
enum class EGameplaySubtitleDisplayTextSize : uint8
{
	ExtraSmall,
	Small,
	Medium,
	Large,
	ExtraLarge,
	
	/** Internal marker for total count */
	MAX
};

/** @brief Preset colors for subtitle text */
UENUM(BlueprintType)
enum class EGameplaySubtitleDisplayTextColor : uint8
{
	White,
	Yellow,
	Cyan,
	Magenta,
	Red,
	Black,
	Blue,
	Green,
	
	/** Internal marker for total count */
	MAX
};

/** @brief Visual styles for the subtitle text outline or shadow */
UENUM(BlueprintType)
enum class EGameplaySubtitleDisplayTextBorder : uint8
{
	/** No outline or shadow */
	None,

	/** Full outline around the text */
	Outline,

	/** Standard drop shadow effect */
	DropShadow,
	
	/** Internal marker for total count */
	MAX
};

/** @brief Opacity levels for the subtitle background box */
UENUM(BlueprintType)
enum class EGameplaySubtitleDisplayBackgroundOpacity : uint8
{
	/** Fully transparent background */
	Clear,
	Low,
	Medium,
	High,
	
	/** Fully opaque background */
	Solid,
	
	/** Internal marker for total count */
	MAX
};

/** @brief Combined formatting options for subtitle display */
USTRUCT(BlueprintType)
struct FGameplaySubtitleFormat
{
	GENERATED_BODY()
	
	FGameplaySubtitleFormat()
		: SubtitleTextSize(EGameplaySubtitleDisplayTextSize::Medium)
		, SubtitleTextColor(EGameplaySubtitleDisplayTextColor::White)
		, SubtitleTextBorder(EGameplaySubtitleDisplayTextBorder::None)
		, SubtitleBackgroundOpacity(EGameplaySubtitleDisplayBackgroundOpacity::Medium)
	{}
	
public:
	/** The size category for the text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	EGameplaySubtitleDisplayTextSize SubtitleTextSize;
	
	/** The color category for the text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	EGameplaySubtitleDisplayTextColor SubtitleTextColor;
	
	/** The border or shadow style for the text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	EGameplaySubtitleDisplayTextBorder SubtitleTextBorder;
	
	/** The opacity of the background box behind the text */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Subtitle")
	EGameplaySubtitleDisplayBackgroundOpacity SubtitleBackgroundOpacity;
};

