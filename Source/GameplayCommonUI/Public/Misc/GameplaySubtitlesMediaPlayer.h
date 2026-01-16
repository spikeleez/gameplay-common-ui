// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Tickable.h"
#include "UObject/Object.h"
#include "GameplaySubtitlesMediaPlayer.generated.h"

class UOverlays;
class UMediaPlayer;
struct FFrame;

/**
 * @brief Logic for playing and synchronizing media subtitles
 * 
 * This object manages the lifecycle and timing of subtitles loaded from an 
 * UOverlays asset. It typically synchronizes its playback time with a 
 * UMediaPlayer to ensure subtitles match the video content.
 * 
 * You should call Play(), Pause(), or Stop() on this object in conjunction 
 * with the corresponding calls on your media player.
 */
UCLASS(BlueprintType)
class GAMEPLAYCOMMONUI_API UGameplaySubtitlesMediaPlayer : public UObject, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	/** The data source containing the subtitle overlay frames */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Subtitles Source")
	TObjectPtr<UOverlays> SourceSubtitles;
	
public:
	UGameplaySubtitlesMediaPlayer(const FObjectInitializer& ObjectInitializer);
	
	//~Begin UObject
	virtual void BeginDestroy() override;
	//~End of UObject
	
	//~Begin FTickableGameObject
	virtual void Tick(float DeltaTime) override;
	virtual ETickableTickType GetTickableTickType() const override { return (HasAnyFlags(RF_ClassDefaultObject) ? ETickableTickType::Never : ETickableTickType::Always); }
	virtual TStatId GetStatId() const override { RETURN_QUICK_DECLARE_CYCLE_STAT(UMediaSubtitlesPlayer, STATGROUP_Tickables); }
	//~End of FTickableGameObject
	
	/** Begins processing and displaying the currently set subtitles */
	UFUNCTION(BlueprintCallable, Category="Subtitles Player")
	void PlaySubtitle();
	
	/** Stops the subtitle playback and clears any active overlays */
	UFUNCTION(BlueprintCallable, Category="Subtitles Player")
	void StopSubtitle();
	
	/** Sets the overlay asset to use for subtitles */
	UFUNCTION(BlueprintCallable, Category="Subtitles Player")
	void SetSubtitle(UOverlays* Subtitles);
	
	/** Binds this subtitle player to a media player to automatically synchronize timing */
	UFUNCTION(BlueprintCallable, Category="Subtitles Player")
	void BindToMediaPlayer(UMediaPlayer* InMediaPlayer);
	
private:
	/** Active media player being synchronized with */
	TWeakObjectPtr<UMediaPlayer> MediaPlayer;
	
	/** Internal flag indicating if subtitle processing is active */
	uint8 bEnabled : 1;
};

