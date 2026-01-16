// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Subsystems/GameplaySubtitlesSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"

UGameplaySubtitlesSubsystem::UGameplaySubtitlesSubsystem()
{
}

void UGameplaySubtitlesSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGameplaySubtitlesSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGameplaySubtitlesSubsystem::SetSubtitleDisplayFormat(const FGameplaySubtitleFormat& InFormat)
{
	SubtitleFormat = InFormat;
	SubtitleFormatChangedEvent.Broadcast(SubtitleFormat);
}

const FGameplaySubtitleFormat& UGameplaySubtitlesSubsystem::GetSubtitleDisplayFormat() const
{
	return SubtitleFormat;
}

UGameplaySubtitlesSubsystem* UGameplaySubtitlesSubsystem::Get(const ULocalPlayer* LocalPlayer)
{
	return LocalPlayer ? LocalPlayer->GetGameInstance()->GetSubsystem<UGameplaySubtitlesSubsystem>() : nullptr;
}
