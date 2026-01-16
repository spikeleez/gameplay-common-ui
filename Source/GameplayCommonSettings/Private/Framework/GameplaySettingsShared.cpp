// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingsShared.h"
#include "Framework/Application/SlateApplication.h"
#include "EnhancedInputSubsystems.h"
#include "Internationalization/Culture.h"
#include "Engine/LocalPlayer.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/App.h"
#include "Subsystems/GameplaySubtitlesSubsystem.h"
#include "UserSettings/EnhancedInputUserSettings.h"
#include "Application/SlateApplicationBase.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingsShared)

/** @brief Global name for the shared settings save slot */
static FString SHARED_SETTINGS_SLOT_NAME = TEXT("SharedGameSettings");

int32 UGameplaySettingsShared::GetLatestDataVersion() const
{
	// 0 = before subclassing ULocalPlayerSaveGame
	// 1 = first proper version
	return 1;
}

UGameplaySettingsShared* UGameplaySettingsShared::CreateTemporarySettings(const ULocalPlayer* InLocalPlayer)
{
	// This is not loaded from disk but should be set up to save
	UGameplaySettingsShared* SharedSettings = Cast<UGameplaySettingsShared>(CreateNewSaveGameForLocalPlayer(UGameplaySettingsShared::StaticClass(), InLocalPlayer, SHARED_SETTINGS_SLOT_NAME));

	SharedSettings->ApplySettings();

	return SharedSettings;
}

UGameplaySettingsShared* UGameplaySettingsShared::LoadOrCreateSettings(const ULocalPlayer* InLocalPlayer)
{
	// This will stall the main thread while it loads
	UGameplaySettingsShared* SharedSettings = Cast<UGameplaySettingsShared>(LoadOrCreateSaveGameForLocalPlayer(UGameplaySettingsShared::StaticClass(), InLocalPlayer, SHARED_SETTINGS_SLOT_NAME));

	SharedSettings->ApplySettings();

	return SharedSettings;
}

bool UGameplaySettingsShared::AsyncLoadOrCreateSettings(const ULocalPlayer* LocalPlayer, FOnSettingsLoadedEvent Delegate)
{
	const FOnLocalPlayerSaveGameLoadedNative Lambda = FOnLocalPlayerSaveGameLoadedNative::CreateLambda([Delegate]
	(ULocalPlayerSaveGame* LoadedSave)
	{
		UGameplaySettingsShared* LoadedSettings = CastChecked<UGameplaySettingsShared>(LoadedSave);
			
		LoadedSettings->ApplySettings();

		Delegate.ExecuteIfBound(LoadedSettings);
	});

	return ULocalPlayerSaveGame::AsyncLoadOrCreateSaveGameForLocalPlayer(UGameplaySettingsShared::StaticClass(), LocalPlayer, SHARED_SETTINGS_SLOT_NAME, Lambda);
}

void UGameplaySettingsShared::SaveSettings()
{
	// Schedule an async save because it's okay if it fails
	AsyncSaveGameToSlotForLocalPlayer();

	// TODO: Move this to the serialize function instead with a bumped version number
	if (const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningPlayer))
	{
		if (UEnhancedInputUserSettings* InputSettings = EnhancedInputSubsystem->GetUserSettings())
		{
			InputSettings->AsyncSaveSettings();
		}
	}
}

void UGameplaySettingsShared::ApplySettings()
{
	ApplyBackgroundAudioSettings();
	ApplySubtitleOptions();
	ApplyCultureSettings();
	
	if (const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(OwningPlayer))
	{
		if (UEnhancedInputUserSettings* InputSettings = EnhancedInputSubsystem->GetUserSettings())
		{
			InputSettings->ApplySettings();
		}
	}
}

void UGameplaySettingsShared::ApplySubtitleOptions()
{
	if (UGameplaySubtitlesSubsystem* SubtitleSystem = UGameplaySubtitlesSubsystem::Get(OwningPlayer))
	{
		FGameplaySubtitleFormat SubtitleFormat;
		SubtitleFormat.SubtitleTextSize = SubtitleTextSize;
		SubtitleFormat.SubtitleTextColor = SubtitleTextColor;
		SubtitleFormat.SubtitleTextBorder = SubtitleTextBorder;
		SubtitleFormat.SubtitleBackgroundOpacity = SubtitleBackgroundOpacity;

		SubtitleSystem->SetSubtitleDisplayFormat(SubtitleFormat);
	}
}

const FString& UGameplaySettingsShared::GetPendingCulture() const
{
	return PendingCulture;
}

void UGameplaySettingsShared::SetPendingCulture(const FString& NewCulture)
{
	PendingCulture = NewCulture;
	bResetToDefaultCulture = false;
	bIsDirty = true;
}

void UGameplaySettingsShared::OnCultureChanged()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}

void UGameplaySettingsShared::ClearPendingCulture()
{
	PendingCulture.Reset();
}

bool UGameplaySettingsShared::IsUsingDefaultCulture() const
{
	FString Culture;
	GConfig->GetString(TEXT("Internationalization"), TEXT("Culture"), Culture, GGameUserSettingsIni);
	
	return Culture.IsEmpty();
}

void UGameplaySettingsShared::ResetToDefaultCulture()
{
	ClearPendingCulture();
	bResetToDefaultCulture = true;
	bIsDirty = true;
}

void UGameplaySettingsShared::ApplyCultureSettings()
{
	if (bResetToDefaultCulture)
	{
		const FCulturePtr SystemDefaultCulture = FInternationalization::Get().GetDefaultCulture();
		check(SystemDefaultCulture.IsValid());
		
		const FString CultureToApply = SystemDefaultCulture->GetName();
		if (FInternationalization::Get().SetCurrentCulture(CultureToApply))
		{
			// Clear this string.
			GConfig->RemoveKey(TEXT("Internationalization"), TEXT("Culture"), GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		}
		bResetToDefaultCulture = false;
	}
	else if (!PendingCulture.IsEmpty())
	{
		// Set Current Culture may trigger PendingCulture to be cleared (if a culture change is broadcast) so we take a copy of it to work with.
		const FString CultureToApply = PendingCulture;
		if (FInternationalization::Get().SetCurrentCulture(CultureToApply))
		{
			// Note: This is intentionally saved to the users config
			// We need to localize text before the player logs in and very early in the loading screen
			GConfig->SetString(TEXT("Internationalization"), TEXT("Culture"), *CultureToApply, GGameUserSettingsIni);
			GConfig->Flush(false, GGameUserSettingsIni);
		}
		ClearPendingCulture();
	}
}

void UGameplaySettingsShared::ResetCultureToCurrentSettings()
{
	ClearPendingCulture();
	bResetToDefaultCulture = false;
}

void UGameplaySettingsShared::SetColorBlindMode(EGameplayColorBlindMode NewColorBlindMode)
{
	if (ColorBlindMode != NewColorBlindMode )
	{
		ColorBlindMode = NewColorBlindMode;
		FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType((EColorVisionDeficiency)(int32)ColorBlindMode, ColorBlindStrength, true, false);
	}
}

void UGameplaySettingsShared::SetColorBlindStrength(int32 NewColorBlindStrength)
{
	NewColorBlindStrength = FMath::Clamp(NewColorBlindStrength, 0, 10);
	if (ColorBlindStrength != NewColorBlindStrength)
	{
		ColorBlindStrength = NewColorBlindStrength;
		FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType((EColorVisionDeficiency)(int32)ColorBlindMode, ColorBlindStrength, true, false);
	}
}

void UGameplaySettingsShared::SetAllowAudioInBackground(EGameplayAllowBackgroundAudio NewValue)
{
	if (ChangeValueAndDirty(AllowAudioInBackground, NewValue))
	{
		ApplyBackgroundAudioSettings();
	}
}

void UGameplaySettingsShared::ApplyBackgroundAudioSettings()
{
	if (OwningPlayer && OwningPlayer->IsPrimaryPlayer())
	{
		FApp::SetUnfocusedVolumeMultiplier((AllowAudioInBackground != EGameplayAllowBackgroundAudio::Off) ? 1.0f : 0.0f);
	}
}

void UGameplaySettingsShared::ApplyInputSensitivity()
{
	
}
