// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Misc/GameplaySubtitlesMediaPlayer.h"
#include "MediaPlayer.h"
#include "Overlays.h"
#include "SubtitleManager.h"

UGameplaySubtitlesMediaPlayer::UGameplaySubtitlesMediaPlayer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, MediaPlayer(nullptr)
	, bEnabled(false)
{
}

void UGameplaySubtitlesMediaPlayer::BeginDestroy()
{
	StopSubtitle();
	
	Super::BeginDestroy();
}

void UGameplaySubtitlesMediaPlayer::Tick(float DeltaTime)
{
	QUICK_SCOPE_CYCLE_COUNTER(STAT_UMediaSubtitlesPlayer_Tick);
	
	if (bEnabled && SourceSubtitles)
	{
		const UMediaPlayer* MediaPlayerPtr = MediaPlayer.Get();
		if (MediaPlayerPtr)
		{
			const FTimespan CurrentTime = MediaPlayerPtr->GetTime();
			TArray<FOverlayItem> CurrentSubtitles;
			SourceSubtitles->GetOverlaysForTime(CurrentTime, CurrentSubtitles);
			
			TArray<FString> SubtitlesText;
			for (const FOverlayItem& Subtitle : CurrentSubtitles)
			{
				SubtitlesText.Add(Subtitle.Text);
			}
			
			FSubtitleManager::GetSubtitleManager()->SetMovieSubtitle(this, SubtitlesText);
		}
		else
		{
			StopSubtitle();
		}
	}
}

void UGameplaySubtitlesMediaPlayer::PlaySubtitle()
{
	bEnabled = true;
}

void UGameplaySubtitlesMediaPlayer::StopSubtitle()
{
	bEnabled = false;
	
	// Clear the movie subtitle for this object.
	FSubtitleManager::GetSubtitleManager()->SetMovieSubtitle(this, TArray<FString>());
}

void UGameplaySubtitlesMediaPlayer::SetSubtitle(UOverlays* Subtitles)
{
	SourceSubtitles = Subtitles;
}

void UGameplaySubtitlesMediaPlayer::BindToMediaPlayer(UMediaPlayer* InMediaPlayer)
{
	MediaPlayer = InMediaPlayer;
}
