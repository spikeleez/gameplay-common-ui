// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Misc/GameplaySubtitlesTypes.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplaySubtitlesSubsystem.generated.h"

class ULocalPlayer;

/**
 * @brief Subsystem for managing global subtitle display settings
 * 
 * This subsystem maintains the current formatting and display preferences 
 * for subtitles (size, color, background, etc.) across the game instance. 
 * It broadcasts events when these settings change so that subtitle widgets 
 * can update accordingly.
 */
UCLASS(DisplayName="Gameplay Subtitles Subsystem")
class GAMEPLAYCOMMONUI_API UGameplaySubtitlesSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
	/** Event fired when the subtitle display format (appearance) is modified */
	DECLARE_EVENT_OneParam(UGameplaySubtitlesSubsystem, FGameplaySubtitleDisplayFormatChangedEvent, const FGameplaySubtitleFormat& /*DisplayFormat*/);
	FGameplaySubtitleDisplayFormatChangedEvent SubtitleFormatChangedEvent;
	
public:
	UGameplaySubtitlesSubsystem();
	
	//~Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem
	
	/** Static helper to get the subtitle subsystem from a local player */
	static UGameplaySubtitlesSubsystem* Get(const ULocalPlayer* LocalPlayer);
	
	/** Updates the global subtitle display format and broadcasts the change event */
	void SetSubtitleDisplayFormat(const FGameplaySubtitleFormat& InFormat);
	
	/** Gets the current global subtitle display format */
	const FGameplaySubtitleFormat& GetSubtitleDisplayFormat() const;
	
private:
	/** Cached subtitle formatting options */
	UPROPERTY()
	FGameplaySubtitleFormat SubtitleFormat;
};

