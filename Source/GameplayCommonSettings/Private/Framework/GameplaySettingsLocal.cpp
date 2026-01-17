// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingsLocal.h"

#include "AudioModulationStatics.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/GameplayCommonPerformanceSettings.h"
#include "ICommonUIModule.h"
#include "Scalability.h"
#include "Engine/Engine.h"
#include "GameplayTagContainer.h"
#include "CommonUISettings.h"
#include "Misc/GameplayCommonSettingsTags.h"
#include "Performance/LatencyMarkerModule.h"
#include "DeviceProfiles/DeviceProfileManager.h"
#include "CommonInputSubsystem.h"
#include "Misc/App.h"
#include "Engine/LocalPlayer.h"
#include "Runtime/Launch/Resources/Version.h"
#include "GameplayCommonSettings.h"
#include "SoundControlBus.h"
#include "SoundControlBusMix.h"
#include "Misc/GameplayCommonAudioSettings.h"
#include "Subsystems/GameplayAudioMixEffectsSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingsLocal)

#if WITH_EDITOR
static TAutoConsoleVariable<bool> CVarApplyFrameRateSettingsInPIE(TEXT("GCS.ApplyFrameRateSettingsInPIE"),
	false,
	TEXT("Should we apply frame rate settings in PIE?"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarApplyFrontEndPerformanceOptionsInPIE(TEXT("GCS.ApplyFrontEndPerformanceOptionsInPIE"),
	false,
	TEXT("Do we apply front-end specific performance options in PIE?"),
	ECVF_Default);

static TAutoConsoleVariable<bool> CVarApplyDeviceProfilesInPIE(TEXT("GCS.ApplyDeviceProfilesInPIE"),
	false,
	TEXT("Should we apply experience/platform emulated device profiles in PIE?"),
	ECVF_Default);
#endif

//////////////////////////////////////////////////////////////////////
// Console frame pacing

static TAutoConsoleVariable<int32> CVarDeviceProfileDrivenTargetFps(
	TEXT("GCS.DeviceProfile.Console.TargetFPS"),
	-1,
	TEXT("Target FPS when being driven by device profile"),
	ECVF_Default | ECVF_Preview);

static TAutoConsoleVariable<int32> CVarDeviceProfileDrivenFrameSyncType(
	TEXT("GCS.DeviceProfile.Console.FrameSyncType"),
	-1,
	TEXT("Sync type when being driven by device profile. Corresponds to r.GTSyncType"),
	ECVF_Default | ECVF_Preview);

namespace GameplayCommonSettingsHelpers
{
	bool HasPlatformTrait(FGameplayTag Tag)
	{
		return ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(Tag);
	}

	// Returns the max level from the integer scalability settings (ignores ResolutionQuality)
	int32 GetHighestLevelOfAnyScalabilityChannel(const Scalability::FQualityLevels& ScalabilityQuality)
	{
		static_assert(sizeof(Scalability::FQualityLevels) == 88, "This function may need to be updated to account for new members.");

		int32 MaxScalability = ScalabilityQuality.ViewDistanceQuality;
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.AntiAliasingQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.ShadowQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.GlobalIlluminationQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.ReflectionQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.PostProcessQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.TextureQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.EffectsQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.FoliageQuality);
		MaxScalability = FMath::Max(MaxScalability, ScalabilityQuality.ShadingQuality);

		return (MaxScalability >= 0) ? MaxScalability : -1;
	}

	void FillScalabilitySettingsFromDeviceProfile(FGameplayScalabilitySnapshot& Mode, const FString& Suffix = FString())
	{
		static_assert(sizeof(Scalability::FQualityLevels) == 88, "This function may need to be updated to account for new members");

		// Default out before filling so we can correctly mark non-overridden scalability values.
		// It's technically possible to swap device profile when testing so safest to clear and refill
		Mode = FGameplayScalabilitySnapshot();

		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.ResolutionQuality%s"), *Suffix), Mode.Qualities.ResolutionQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.ViewDistanceQuality%s"), *Suffix), Mode.Qualities.ViewDistanceQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.AntiAliasingQuality%s"), *Suffix), Mode.Qualities.AntiAliasingQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.ShadowQuality%s"), *Suffix), Mode.Qualities.ShadowQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.GlobalIlluminationQuality%s"), *Suffix), Mode.Qualities.GlobalIlluminationQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.ReflectionQuality%s"), *Suffix), Mode.Qualities.ReflectionQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.PostProcessQuality%s"), *Suffix), Mode.Qualities.PostProcessQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.TextureQuality%s"), *Suffix), Mode.Qualities.TextureQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.EffectsQuality%s"), *Suffix), Mode.Qualities.EffectsQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.FoliageQuality%s"), *Suffix), Mode.Qualities.FoliageQuality);
		Mode.bHasOverrides |= UDeviceProfileManager::GetScalabilityCVar(FString::Printf(TEXT("sg.ShadingQuality%s"), *Suffix), Mode.Qualities.ShadingQuality);
	}

	void OverrideQualityLevelsToScalabilityMode(const FGameplayScalabilitySnapshot& InMode, Scalability::FQualityLevels& InOutLevels)
	{
		static_assert(sizeof(Scalability::FQualityLevels) == 88, "This function may need to be updated to account for new members");

		// Overrides any valid (non-negative) settings
		InOutLevels.ResolutionQuality = (InMode.Qualities.ResolutionQuality >= 0.f) ? InMode.Qualities.ResolutionQuality : InOutLevels.ResolutionQuality;
		InOutLevels.ViewDistanceQuality = (InMode.Qualities.ViewDistanceQuality >= 0) ? InMode.Qualities.ViewDistanceQuality : InOutLevels.ViewDistanceQuality;
		InOutLevels.AntiAliasingQuality = (InMode.Qualities.AntiAliasingQuality >= 0) ? InMode.Qualities.AntiAliasingQuality : InOutLevels.AntiAliasingQuality;
		InOutLevels.ShadowQuality = (InMode.Qualities.ShadowQuality >= 0) ? InMode.Qualities.ShadowQuality : InOutLevels.ShadowQuality;
		InOutLevels.GlobalIlluminationQuality = (InMode.Qualities.GlobalIlluminationQuality >= 0) ? InMode.Qualities.GlobalIlluminationQuality : InOutLevels.GlobalIlluminationQuality;
		InOutLevels.ReflectionQuality = (InMode.Qualities.ReflectionQuality >= 0) ? InMode.Qualities.ReflectionQuality : InOutLevels.ReflectionQuality;
		InOutLevels.PostProcessQuality = (InMode.Qualities.PostProcessQuality >= 0) ? InMode.Qualities.PostProcessQuality : InOutLevels.PostProcessQuality;
		InOutLevels.TextureQuality = (InMode.Qualities.TextureQuality >= 0) ? InMode.Qualities.TextureQuality : InOutLevels.TextureQuality;
		InOutLevels.EffectsQuality = (InMode.Qualities.EffectsQuality >= 0) ? InMode.Qualities.EffectsQuality : InOutLevels.EffectsQuality;
		InOutLevels.FoliageQuality = (InMode.Qualities.FoliageQuality >= 0) ? InMode.Qualities.FoliageQuality : InOutLevels.FoliageQuality;
		InOutLevels.ShadingQuality = (InMode.Qualities.ShadingQuality >= 0) ? InMode.Qualities.ShadingQuality : InOutLevels.ShadingQuality;
	}
}

// Combines two limits, always taking the minimum of the two (with special handling for values of <= 0 meaning unlimited)
float CombineFrameRateLimits(float Limit1, float Limit2)
{
	if (Limit1 <= 0.0f)
	{
		return Limit2;
	}
	
	if (Limit2 <= 0.0f)
	{
		return Limit1;
	}
	
	return FMath::Min(Limit1, Limit2);
}

FGameplayScalabilitySnapshot::FGameplayScalabilitySnapshot()
{
	static_assert(sizeof(Scalability::FQualityLevels) == 88, "This function may need to be updated to account for new members");

	Qualities.ResolutionQuality = -1.0f;
	Qualities.ViewDistanceQuality = -1;
	Qualities.AntiAliasingQuality = -1;
	Qualities.ShadowQuality = -1;
	Qualities.GlobalIlluminationQuality = -1;
	Qualities.ReflectionQuality = -1;
	Qualities.PostProcessQuality = -1;
	Qualities.TextureQuality = -1;
	Qualities.EffectsQuality = -1;
	Qualities.FoliageQuality = -1;
	Qualities.ShadingQuality = -1;
}

UGameplaySettingsLocal::UGameplaySettingsLocal()
{
	if (!HasAnyFlags(RF_ClassDefaultObject) && FSlateApplication::IsInitialized())
	{
		OnApplicationActivationStateChangedHandle = FSlateApplication::Get().OnApplicationActivationStateChanged().AddUObject(this, &ThisClass::OnAppActivationStateChanged);
	}

#if FROM_ENGINE_VERSION(5, 5)
	bEnableScalabilitySettings = UGameplayPlatformSpecificRenderingSettings::Get()->bSupportsGranularVideoQualitySettings;
#endif
	
	SetToDefaults();
}

void UGameplaySettingsLocal::BeginDestroy()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().OnApplicationActivationStateChanged().Remove(OnApplicationActivationStateChangedHandle);
	}

	Super::BeginDestroy();
}

void UGameplaySettingsLocal::SetToDefaults()
{
	Super::SetToDefaults();

	bUseHeadphoneMode = false;
	bUseHDRAudioMode = false;
	bSoundControlBusMixLoaded = false;
	bEnableLatencyTrackingStats = UGameplaySettingsLocal::DoesPlatformSupportLatencyTrackingStats();

	const UGameplayPlatformSpecificRenderingSettings* PlatformSettings = UGameplayPlatformSpecificRenderingSettings::Get();
	UserChosenDeviceProfileSuffix = PlatformSettings->DefaultDeviceProfileSuffix;
	DesiredUserChosenDeviceProfileSuffix = UserChosenDeviceProfileSuffix;

	FrameRateLimit_InMenu = 144.0f;
	FrameRateLimit_WhenBackgrounded = 30.0f;
	FrameRateLimit_OnBattery = 60.0f;

	/*MobileFrameRateLimit = GetDefaultMobileFrameRate();
	DesiredMobileFrameRateLimit = MobileFrameRateLimit;*/
}

void UGameplaySettingsLocal::LoadSettings(bool bForceReload)
{
	Super::LoadSettings(bForceReload);
	
	// Console platforms use rhi.SyncInterval to limit framerate
	const UGameplayPlatformSpecificRenderingSettings* PlatformSettings = UGameplayPlatformSpecificRenderingSettings::Get();
	if (PlatformSettings->FramePacingMode == EGameplayFramePacingMode::ConsoleStyle)
	{
		FrameRateLimit = 0.0f;
	}

	// Enable HRTF if needed
	bDesiredHeadphoneMode = bUseHeadphoneMode;
	SetHeadphoneModeEnabled(bUseHeadphoneMode);

	ApplyLatencyTrackingStatSetting();

	DesiredUserChosenDeviceProfileSuffix = UserChosenDeviceProfileSuffix;

	GameplayCommonSettingsHelpers::FillScalabilitySettingsFromDeviceProfile(DeviceDefaultScalabilitySettings);

	/*DesiredMobileFrameRateLimit = MobileFrameRateLimit;
	ClampMobileQuality();*/

	PerfStatSettingsChangedEvent.Broadcast();
}

void UGameplaySettingsLocal::ResetToCurrentSettings()
{
	Super::ResetToCurrentSettings();

	bDesiredHeadphoneMode = bUseHeadphoneMode;

	UserChosenDeviceProfileSuffix = DesiredUserChosenDeviceProfileSuffix;

	//MobileFrameRateLimit = DesiredMobileFrameRateLimit;
}

void UGameplaySettingsLocal::ConfirmVideoMode()
{
	Super::ConfirmVideoMode();

	//SetMobileFPSMode(DesiredMobileFrameRateLimit);
}

float UGameplaySettingsLocal::GetEffectiveFrameRateLimit()
{
	const UGameplayPlatformSpecificRenderingSettings* PlatformSettings = UGameplayPlatformSpecificRenderingSettings::Get();

#if WITH_EDITOR
	if (GIsEditor && !CVarApplyFrameRateSettingsInPIE.GetValueOnGameThread())
	{
		return Super::GetEffectiveFrameRateLimit();
	}
#endif

	if (PlatformSettings->FramePacingMode == EGameplayFramePacingMode::ConsoleStyle)
	{
		return 0.0f;
	}

	float EffectiveFrameRateLimit = Super::GetEffectiveFrameRateLimit();

	if (ShouldUseFrontendPerformanceSettings())
	{
		EffectiveFrameRateLimit = CombineFrameRateLimits(EffectiveFrameRateLimit, FrameRateLimit_InMenu);
	}

	if (PlatformSettings->FramePacingMode == EGameplayFramePacingMode::DesktopStyle)
	{
		if (FPlatformMisc::IsRunningOnBattery())
		{
			EffectiveFrameRateLimit = CombineFrameRateLimits(EffectiveFrameRateLimit, FrameRateLimit_OnBattery);
		}

		if (FSlateApplication::IsInitialized() && !FSlateApplication::Get().IsActive())
		{
			EffectiveFrameRateLimit = CombineFrameRateLimits(EffectiveFrameRateLimit, FrameRateLimit_WhenBackgrounded);
		}
	}

	return EffectiveFrameRateLimit;
}	

void UGameplaySettingsLocal::ApplyNonResolutionSettings()
{
	Super::ApplyNonResolutionSettings();

	// Check if Control Bus Mix references have been loaded,
	// Might be false if applying non resolution settings without touching any of the setters from UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	// In this section, update each Control Bus to the currently cached UI settings
	{
		if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Overall")))
		{
			if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
			{
				SetVolumeForControlBus(ControlBusPtr, OverallVolume);
			}
		}

		if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Music")))
		{
			if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
			{
				SetVolumeForControlBus(ControlBusPtr, MusicVolume);
			}
		}

		if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("SoundEffects")))
		{
			if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
			{
				SetVolumeForControlBus(ControlBusPtr, SoundEffectsVolume);
			}
		}

		if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Dialogue")))
		{
			if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
			{
				SetVolumeForControlBus(ControlBusPtr, DialogueVolume);
			}
		}

		if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("VoiceChat")))
		{
			if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
			{
				SetVolumeForControlBus(ControlBusPtr, VoiceChatVolume);
			}
		}
	}

	if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetTypedOuter<ULocalPlayer>()))
	{
		InputSubsystem->SetGamepadInputType(ControllerPlatform);
	}

	if (bUseHeadphoneMode != bDesiredHeadphoneMode)
	{
		SetHeadphoneModeEnabled(bDesiredHeadphoneMode);
	}

	if (DesiredUserChosenDeviceProfileSuffix != UserChosenDeviceProfileSuffix)
	{
		UserChosenDeviceProfileSuffix = DesiredUserChosenDeviceProfileSuffix;
	}

	if (FApp::CanEverRender())
	{
		ApplyDisplayGamma();
		//ApplySafeZoneScale();
		//UpdateGameModeDeviceProfileAndFps();
	}

	PerfStatSettingsChangedEvent.Broadcast();
}

int32 UGameplaySettingsLocal::GetOverallScalabilityLevel() const
{
	int32 Result = Super::GetOverallScalabilityLevel();

	const UGameplayPlatformSpecificRenderingSettings* PlatformSettings = UGameplayPlatformSpecificRenderingSettings::Get();
	if (PlatformSettings->FramePacingMode == EGameplayFramePacingMode::MobileStyle)
	{
		Result = GameplayCommonSettingsHelpers::GetHighestLevelOfAnyScalabilityChannel(ScalabilityQuality);
	}

	return Result;
}

void UGameplaySettingsLocal::SetOverallScalabilityLevel(int32 Value)
{
	TGuardValue Guard(bSettingOverallQualityGuard, true);

	Value = FMath::Clamp(Value, 0, 3);

	float CurrentMobileResolutionQuality = ScalabilityQuality.ResolutionQuality;

	Super::SetOverallScalabilityLevel(Value);

	const UGameplayPlatformSpecificRenderingSettings* PlatformSettings = UGameplayPlatformSpecificRenderingSettings::Get();
	if (PlatformSettings->FramePacingMode == EGameplayFramePacingMode::MobileStyle)
	{
		// Restore the resolution quality, mobile decouples this from overall quality
		ScalabilityQuality.ResolutionQuality = CurrentMobileResolutionQuality;

		// Changing the overall quality can end up adjusting the frame rate on mobile since there are limits
		/*const int32 ConstrainedFrameRateLimit = GameplayCommonSettingsHelpers::ConstrainFrameRateToBeCompatibleWithOverallQuality(DesiredMobileFrameRateLimit, Value);
		if (ConstrainedFrameRateLimit != DesiredMobileFrameRateLimit)
		{
			SetDesiredMobileFrameRateLimit(ConstrainedFrameRateLimit);
		}*/
	}
}

void UGameplaySettingsLocal::OnAppActivationStateChanged(bool bIsActive)
{
	// We might want to adjust the frame rate when the app loses/gains focus on multi-window platforms
	UpdateEffectiveFrameRateLimit();
}

void UGameplaySettingsLocal::ReapplyThingsDueToPossibleDeviceProfileChange()
{
	ApplyNonResolutionSettings();
}

void UGameplaySettingsLocal::SetShouldUseFrontendPerformanceSettings(bool bInFrontEnd)
{
	bInFrontEndForPerformancePurposes = bInFrontEnd;
	UpdateEffectiveFrameRateLimit();
}

bool UGameplaySettingsLocal::ShouldUseFrontendPerformanceSettings() const
{
#if WITH_EDITOR
	if (GIsEditor && !CVarApplyFrontEndPerformanceOptionsInPIE.GetValueOnGameThread())
	{
		return false;
	}
#endif

	return bInFrontEndForPerformancePurposes;
}

UGameplaySettingsLocal* UGameplaySettingsLocal::Get()
{
	return GEngine ? CastChecked<UGameplaySettingsLocal>(GEngine->GetGameUserSettings()) : nullptr;
}

void UGameplaySettingsLocal::ApplyScalabilitySettings()
{
	Scalability::SetQualityLevels(ScalabilityQuality);
}

int32 UGameplaySettingsLocal::GetFirstFrameRateWithQualityLimit() const
{
	return 0;
}

int32 UGameplaySettingsLocal::GetLowestQualityWithFrameRateLimit() const
{
	return 0;
}

int32 UGameplaySettingsLocal::GetMaxSupportedOverallQualityLevel() const
{
	//return GameplayCommonSettingsHelpers::GetHighestLevelOfAnyScalabilityChannel(DeviceDefaultScalabilitySettings.Qualities);

	return 3;
}

bool UGameplaySettingsLocal::DoesPlatformSupportLatencyTrackingStats()
{
	return ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(GameplayCommonSettingsTags::Trait_SupportsLatencyStats);
}

EGameplayPerformanceStatDisplayMode UGameplaySettingsLocal::GetPerfStatDisplayState(EGameplayDisplayablePerformanceStat Stat) const
{
	if (const EGameplayPerformanceStatDisplayMode* pMode = DisplayStatList.Find(Stat))
	{
		return *pMode;
	}
	else
	{
		return EGameplayPerformanceStatDisplayMode::Hidden;
	}
}

void UGameplaySettingsLocal::SetPerfStatDisplayState(EGameplayDisplayablePerformanceStat Stat, EGameplayPerformanceStatDisplayMode DisplayMode)
{
	if (DisplayMode == EGameplayPerformanceStatDisplayMode::Hidden)
	{
		DisplayStatList.Remove(Stat);
	}
	else
	{
		DisplayStatList.FindOrAdd(Stat) = DisplayMode;
	}
	PerfStatSettingsChangedEvent.Broadcast();
}

bool UGameplaySettingsLocal::DoesPlatformSupportLatencyMarkers()
{
	return ICommonUIModule::GetSettings().GetPlatformTraits().HasTag(GameplayCommonSettingsTags::Trait_SupportsLatencyMarkers);
}

void UGameplaySettingsLocal::SetEnableLatencyFlashIndicators(const bool bNewVal)
{
	if (bNewVal != bEnableLatencyFlashIndicators)
	{
		bEnableLatencyFlashIndicators = bNewVal;
		LatencyFlashIndicatorSettingsChangedEvent.Broadcast();
	}
}

void UGameplaySettingsLocal::SetEnableLatencyTrackingStats(const bool bNewVal)
{
	if (bNewVal != bEnableLatencyTrackingStats)
	{
		bEnableLatencyTrackingStats = bNewVal;

		ApplyLatencyTrackingStatSetting();

		LatencyStatIndicatorSettingsChangedEvent.Broadcast();
	}
}

void UGameplaySettingsLocal::SetControllerPlatform(const FName InControllerPlatform)
{
	if (ControllerPlatform != InControllerPlatform)
	{
		ControllerPlatform = InControllerPlatform;

		// Apply the change to the common input subsystem so that we refresh any input icons we're using.
		if (UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(GetTypedOuter<ULocalPlayer>()))
		{
			InputSubsystem->SetGamepadInputType(ControllerPlatform);
		}
	}
}

FName UGameplaySettingsLocal::GetControllerPlatform() const
{
	return ControllerPlatform;
}

void UGameplaySettingsLocal::SetHDRAudioMode(bool bEnabled)
{
	bUseHDRAudioMode = bEnabled;

	if (GEngine)
	{
		if (const UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			if (UGameplayAudioMixEffectsSubsystem* GameplayAudioMixEffectsSubsystem = World->GetSubsystem<UGameplayAudioMixEffectsSubsystem>())
			{
				GameplayAudioMixEffectsSubsystem->ApplyDynamicRangeEffectsChains(bEnabled);
			}
		}
	}
}

void UGameplaySettingsLocal::SetDesiredHeadphoneMode(bool bEnabled)
{
	bUseHeadphoneMode = bEnabled;
}

void UGameplaySettingsLocal::SetHeadphoneModeEnabled(bool bEnabled)
{
	if (CanModifyHeadphoneModeEnabled())
	{
		static IConsoleVariable* BinauralSpatializationDisabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("au.DisableBinauralSpatialization"));
		if (BinauralSpatializationDisabledCVar)
		{
			BinauralSpatializationDisabledCVar->Set(!bEnabled, ECVF_SetByGameSetting);

			// Only save settings if the setting actually changed
			if (bUseHeadphoneMode != bEnabled)
			{
				bUseHeadphoneMode = bEnabled;
				SaveSettings();
			}
		}
	}
}

bool UGameplaySettingsLocal::CanModifyHeadphoneModeEnabled() const
{
	static IConsoleVariable* BinauralSpatializationDisabledCVar = IConsoleManager::Get().FindConsoleVariable(TEXT("au.DisableBinauralSpatialization"));
	const bool bHRTFOptionAvailable = BinauralSpatializationDisabledCVar && ((BinauralSpatializationDisabledCVar->GetFlags() & EConsoleVariableFlags::ECVF_SetByMask) <= EConsoleVariableFlags::ECVF_SetByGameSetting);
	const bool bBinauralSettingControlledByOS = GameplayCommonSettingsHelpers::HasPlatformTrait(GameplayCommonSettingsTags::Trait_BinauralSettingControlledByOS);

	return bHRTFOptionAvailable && !bBinauralSettingControlledByOS;
}

float UGameplaySettingsLocal::GetOverallVolume() const
{
	return OverallVolume;
}

void UGameplaySettingsLocal::SetOverallVolume(float NewVolume)
{
	// Cache the incoming volume value.
	OverallVolume = NewVolume;
	
	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called from the UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}
	
	// Ensure it's been loaded before continuing.
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));
	
	// Locate the locally cached bus and set the volume on it.
	if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Overall")))
	{
		if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
		{
			SetVolumeForControlBus(ControlBusPtr, OverallVolume);
		}
	}
}

void UGameplaySettingsLocal::LoadUserControlBusMix()
{
	if (GEngine)
	{
		if (const UWorld* World = GEngine->GetCurrentPlayWorld())
		{
			if (const UGameplayCommonAudioSettings* GameplayAudioSettings = GetDefault<UGameplayCommonAudioSettings>())
			{
				USoundControlBus* OverallControlBus = nullptr;
				USoundControlBus* MusicControlBus = nullptr;
				USoundControlBus* SoundFXControlBus = nullptr;
				USoundControlBus* DialogueControlBus = nullptr;
				USoundControlBus* VoiceChatControlBus = nullptr;

				ControlBusMap.Empty();

				if (UObject* ObjPath = GameplayAudioSettings->OverallVolumeControlBus.TryLoad())
				{
					if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
					{
						OverallControlBus = SoundControlBus;
						ControlBusMap.Add(TEXT("Overall"), OverallControlBus);
					}
					else
					{
						ensureMsgf(SoundControlBus, TEXT("Overall Control Bus reference missing from Lyra Audio Settings."));
					}
				}

				if (UObject* ObjPath = GameplayAudioSettings->MusicVolumeControlBus.TryLoad())
				{
					if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
					{
						MusicControlBus = SoundControlBus;
						ControlBusMap.Add(TEXT("Music"), MusicControlBus);
					}
					else
					{
						ensureMsgf(SoundControlBus, TEXT("Music Control Bus reference missing from Lyra Audio Settings."));
					}
				}

				if (UObject* ObjPath = GameplayAudioSettings->SoundEffectsVolumeControlBus.TryLoad())
				{
					if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
					{
						SoundFXControlBus = SoundControlBus;
						ControlBusMap.Add(TEXT("SoundFX"), SoundFXControlBus);
					}
					else
					{
						ensureMsgf(SoundControlBus, TEXT("SoundFX Control Bus reference missing from Lyra Audio Settings."));
					}
				}

				if (UObject* ObjPath = GameplayAudioSettings->DialogueVolumeControlBus.TryLoad())
				{
					if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
					{
						DialogueControlBus = SoundControlBus;
						ControlBusMap.Add(TEXT("Dialogue"), DialogueControlBus);
					}
					else
					{
						ensureMsgf(SoundControlBus, TEXT("Dialogue Control Bus reference missing from Lyra Audio Settings."));
					}
				}

				if (UObject* ObjPath = GameplayAudioSettings->VoiceChatVolumeControlBus.TryLoad())
				{
					if (USoundControlBus* SoundControlBus = Cast<USoundControlBus>(ObjPath))
					{
						VoiceChatControlBus = SoundControlBus;
						ControlBusMap.Add(TEXT("VoiceChat"), VoiceChatControlBus);
					}
					else
					{
						ensureMsgf(SoundControlBus, TEXT("VoiceChat Control Bus reference missing from Lyra Audio Settings."));
					}
				}

				if (UObject* ObjPath = GameplayAudioSettings->UserSettingsControlBusMix.TryLoad())
				{
					if (USoundControlBusMix* SoundControlBusMix = Cast<USoundControlBusMix>(ObjPath))
					{
						ControlBusMix = SoundControlBusMix;

						const FSoundControlBusMixStage OverallControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, OverallControlBus, OverallVolume);
						const FSoundControlBusMixStage MusicControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, MusicControlBus, MusicVolume);
						const FSoundControlBusMixStage SoundFXControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, SoundFXControlBus, SoundEffectsVolume);
						const FSoundControlBusMixStage DialogueControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, DialogueControlBus, DialogueVolume);
						const FSoundControlBusMixStage VoiceChatControlBusMixStage = UAudioModulationStatics::CreateBusMixStage(World, VoiceChatControlBus, VoiceChatVolume);

						TArray<FSoundControlBusMixStage> ControlBusMixStageArray;
						ControlBusMixStageArray.Add(OverallControlBusMixStage);
						ControlBusMixStageArray.Add(MusicControlBusMixStage);
						ControlBusMixStageArray.Add(SoundFXControlBusMixStage);
						ControlBusMixStageArray.Add(DialogueControlBusMixStage);
						ControlBusMixStageArray.Add(VoiceChatControlBusMixStage);

						UAudioModulationStatics::UpdateMix(World, ControlBusMix, ControlBusMixStageArray);

						bSoundControlBusMixLoaded = true;
					}
					else
					{
						ensureMsgf(SoundControlBusMix, TEXT("User Settings Control Bus Mix reference missing from Lyra Audio Settings."));
					}
				}
			}
		}
	}
}

void UGameplaySettingsLocal::SetVolumeForControlBus(USoundControlBus* ControlBus, float NewVolume)
{
	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}
	
	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));
	
	// Assuming everything has been loaded correctly, we retrieve the world and use AudioModulationStatics to update the Control Bus Volume values and
	// apply the settings to the cached User Control Bus Mix
	if (GEngine && ControlBus && bSoundControlBusMixLoaded)
	{
		if (const UWorld* AudioWorld = GEngine->GetCurrentPlayWorld())
		{
			ensureMsgf(ControlBusMix, TEXT("Control Bus Mix failed to load."));

			// Create and set the Control Bus Mix Stage Parameters
			FSoundControlBusMixStage UpdatedControlBusMixStage;
			UpdatedControlBusMixStage.Bus = ControlBus;
			UpdatedControlBusMixStage.Value.TargetValue = NewVolume;
			UpdatedControlBusMixStage.Value.AttackTime = 0.01f;
			UpdatedControlBusMixStage.Value.ReleaseTime = 0.01f;

			// Add the Control Bus Mix Stage to an Array as the UpdateMix function requires
			TArray<FSoundControlBusMixStage> UpdatedMixStageArray;
			UpdatedMixStageArray.Add(UpdatedControlBusMixStage);

			// Modify the matching bus Mix Stage parameters on the User Control Bus Mix
			UAudioModulationStatics::UpdateMix(AudioWorld, ControlBusMix, UpdatedMixStageArray);
		}
	}
}

float UGameplaySettingsLocal::GetMusicVolume() const
{
	return MusicVolume;
}

void UGameplaySettingsLocal::SetMusicVolume(float NewVolume)
{
	// Cache the incoming volume value
	MusicVolume = NewVolume;

	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called from the UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	// Ensure it's been loaded before continuing.
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Locate the locally cached bus and set the volume on it.
	if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Music")))
	{
		if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
		{
			SetVolumeForControlBus(ControlBusPtr, MusicVolume);
		}
	}
}

float UGameplaySettingsLocal::GetSoundEffectsVolume() const
{
	return SoundEffectsVolume;
}

void UGameplaySettingsLocal::SetSoundEffectsVolume(float NewVolume)
{
	// Cache the incoming volume value
	SoundEffectsVolume = NewVolume;

	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called from the UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Locate the locally cached bus and set the volume on it
	if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("SoundFX")))
	{
		if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
		{
			SetVolumeForControlBus(ControlBusPtr, SoundEffectsVolume);
		}
	}
}

float UGameplaySettingsLocal::GetDialogueVolume() const
{
	return DialogueVolume;
}

void UGameplaySettingsLocal::SetDialogueVolume(float NewVolume)
{
	// Cache the incoming volume value
	DialogueVolume = NewVolume;

	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called from the UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Locate the locally cached bus and set the volume on it
	if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("Dialogue")))
	{
		if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
		{
			SetVolumeForControlBus(ControlBusPtr, DialogueVolume);
		}
	}
}

float UGameplaySettingsLocal::GetVoiceChatVolume() const
{
	return VoiceChatVolume;
}

void UGameplaySettingsLocal::SetVoiceChatVolume(float NewVolume)
{
	// Cache the incoming volume value
	VoiceChatVolume = NewVolume;

	// Check to see if references to the control buses and control bus mixes have been loaded yet
	// Will likely need to be loaded if this function is the first time a setter has been called from the UI
	if (!bSoundControlBusMixLoaded)
	{
		LoadUserControlBusMix();
	}

	// Ensure it's been loaded before continuing
	ensureMsgf(bSoundControlBusMixLoaded, TEXT("UserControlBusMix Settings Failed to Load."));

	// Locate the locally cached bus and set the volume on it
	if (const TObjectPtr<USoundControlBus>* ControlBusDblPtr = ControlBusMap.Find(TEXT("VoiceChat")))
	{
		if (USoundControlBus* ControlBusPtr = *ControlBusDblPtr)
		{
			SetVolumeForControlBus(ControlBusPtr, VoiceChatVolume);
		}
	}
}

void UGameplaySettingsLocal::SetAudioOutputDeviceId(const FString& NewAudioOutputDeviceId)
{
	if (AudioOutputDeviceId != NewAudioOutputDeviceId)
	{
		AudioOutputDeviceId = NewAudioOutputDeviceId;
		if (OnAudioOutputDeviceChanged.IsBound())
		{
			OnAudioOutputDeviceChanged.Broadcast(NewAudioOutputDeviceId);
		}
	}
}

bool UGameplaySettingsLocal::CanRunAutoBenchmark() const
{
	const UGameplayPlatformSpecificRenderingSettings* PlatformSettings = UGameplayPlatformSpecificRenderingSettings::Get();
	return PlatformSettings->bSupportsAutomaticVideoQualityBenchmark;
}

bool UGameplaySettingsLocal::ShouldRunAutoBenchmarkAtStartup() const
{
	if (!CanRunAutoBenchmark())
	{
		return false;
	}

	if (LastCPUBenchmarkResult != -1)
	{
		// Already run and loaded.
		return false;
	}

	return true;
}

void UGameplaySettingsLocal::RunAutoBenchmark(bool bSaveImmediately)
{
	RunHardwareBenchmark();

	// Always apply, optionally save.
	ApplyScalabilitySettings();
	ApplyLatencyTrackingStatSetting();

	if (bSaveImmediately)
	{
		SaveSettings();
	}
}

float UGameplaySettingsLocal::GetFrameRateLimit_OnBattery() const
{
	return FrameRateLimit_OnBattery;
}

void UGameplaySettingsLocal::SetFrameRateLimit_OnBattery(float NewLimitFPS)
{
	FrameRateLimit_OnBattery = NewLimitFPS;
	UpdateEffectiveFrameRateLimit();
}

float UGameplaySettingsLocal::GetFrameRateLimit_InMenu() const
{
	return FrameRateLimit_InMenu;
}

void UGameplaySettingsLocal::SetFrameRateLimit_InMenu(float NewLimitFPS)
{
	FrameRateLimit_InMenu = NewLimitFPS;
	UpdateEffectiveFrameRateLimit();
}

float UGameplaySettingsLocal::GetFrameRateLimit_WhenBackgrounded() const
{
	return FrameRateLimit_WhenBackgrounded;
}

void UGameplaySettingsLocal::SetFrameRateLimit_WhenBackgrounded(float NewLimitFPS)
{
	FrameRateLimit_WhenBackgrounded = NewLimitFPS;
	UpdateEffectiveFrameRateLimit();
}

float UGameplaySettingsLocal::GetFrameRateLimit_Always() const
{
	return GetFrameRateLimit();
}

void UGameplaySettingsLocal::SetFrameRateLimit_Always(float NewLimitFPS)
{
	SetFrameRateLimit(NewLimitFPS);
	UpdateEffectiveFrameRateLimit();
}

void UGameplaySettingsLocal::UpdateEffectiveFrameRateLimit()
{
	if (!IsRunningDedicatedServer())
	{
		SetFrameRateLimitCVar(GetEffectiveFrameRateLimit());
	}
}

void UGameplaySettingsLocal::ApplyLatencyTrackingStatSetting()
{
	// Since this function will be called on load of the settings, we check if the slate app is initalized.
	// If it isn't then we are not in a target which can even have latency stats (like a headless cooker) so we
	// will exit early and do nothing.
	if (!FSlateApplication::IsInitialized())
	{
		return;
	}

	// Don't bother doing anything if the platform doesn't even support tracking stats.
	if (!DoesPlatformSupportLatencyTrackingStats())
	{
		return;
	}

	// Actually enable or disable the latency marker modules based on this setting
	TArray<ILatencyMarkerModule*> LatencyMarkerModules = IModularFeatures::Get().GetModularFeatureImplementations<ILatencyMarkerModule>(ILatencyMarkerModule::GetModularFeatureName());
	for (ILatencyMarkerModule* LatencyMarkerModule : LatencyMarkerModules)
	{
		LatencyMarkerModule->SetEnabled(bEnableLatencyTrackingStats);
	}

	UE_CLOG(!LatencyMarkerModules.IsEmpty(),
		LogConsoleResponse,
		Log,
		TEXT("%s %d Latency Marker Module(s)"),
		bEnableLatencyTrackingStats ? TEXT("Enabled") : TEXT("Disabled"), LatencyMarkerModules.Num());
}

float UGameplaySettingsLocal::GetDisplayGamma() const
{
	return DisplayGamma;
}

void UGameplaySettingsLocal::SetDisplayGamma(float InValue)
{
	DisplayGamma = InValue;
	ApplyDisplayGamma();
}

void UGameplaySettingsLocal::ApplyDisplayGamma()
{
	if (GEngine)
	{
		GEngine->DisplayGamma = DisplayGamma;
	}
}
