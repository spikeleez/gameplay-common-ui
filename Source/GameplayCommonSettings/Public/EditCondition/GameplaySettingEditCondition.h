// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/GameplaySetting.h"
#include "Interfaces/GameplayCommonSettingsInterface.h"
#include "Misc/GameplayCommonPerformanceSettings.h"
#include "HAL/IConsoleManager.h"
#include "RHI.h"
#include "Framework/GameplaySettingsLocal.h"

#define LOCTEXT_NAMESPACE "SettingEditCondition"

class FGameplaySettingEditableState;
class UGameplaySetting;

static TAutoConsoleVariable<bool> CVarLatencyMarkersRequireNVIDIA(TEXT("GameplayCommonSettings.LatencyMarkersRequireNVIDIA"),
	true,
	TEXT("If true, then only allow latency markers to be enabled on NVIDIA hardware"),
	ECVF_Default
);

/**
 * @brief Specifies the matching behavior for frame pacing edit conditions
 */
enum class EGameplayFramePacingEditCondition
{
	/** Enable the setting if the condition matches */
	EnableIf,

	/** Disable the setting if the condition matches */
	DisableIf
};

/**
 * @brief Base class for setting edit conditions
 * 
 * FGameplaySettingEditCondition objects are used to dynamically control the visibility
 * and editability of a gameplay setting. They can listen for external events or 
 * context changes and update the setting's state accordingly.
 * 
 * Edit conditions are shared references and can be added to multiple settings.
 * 
 * @see UGameplaySetting::AddEditCondition
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingEditCondition : public TSharedFromThis<FGameplaySettingEditCondition>
{
public:
	FGameplaySettingEditCondition() { }
	virtual ~FGameplaySettingEditCondition() { }

	/** Event broadcasted when the edit condition's state might have changed */
	DECLARE_EVENT_OneParam(FGameplaySettingEditCondition, FGameplayOnEditConditionChanged, bool);
	FGameplayOnEditConditionChanged OnEditConditionChanged;

	/** 
	 * @brief Broadcasts the OnEditConditionChanged event
	 * 
	 * Call this when internal state changes that should trigger a re-evaluation
	 * of the settings that use this condition.
	 */
	void BroadcastEditConditionChanged()
	{
		OnEditConditionChanged.Broadcast(true);
	}

	/** 
	 * @brief Called during the setting initialization
	 * @param InLocalPlayer The local player context
	 */
	virtual void Initialize(const ULocalPlayer* InLocalPlayer)
	{
	}

	/** 
	 * @brief Called when the setting is 'applied'
	 * @param InLocalPlayer The local player context
	 * @param InSetting The setting being applied
	 */
	virtual void SettingApplied(const ULocalPlayer* InLocalPlayer, UGameplaySetting* InSetting) const
	{
	}

	/** 
	 * @brief Called when the setting is 'changed'
	 * @param InLocalPlayer The local player context
	 * @param InSetting The setting that changed
	 * @param InReason The reason for the change
	 */
	virtual void SettingChanged(const ULocalPlayer* InLocalPlayer, UGameplaySetting* InSetting, EGameplaySettingChangeReason InReason) const
	{
	}

	/**
	 * @brief Evaluates the edit state of a setting
	 * 
	 * Subclasses should implement this to modify InOutEditState based on 
	 * current game state or platform conditions.
	 * 
	 * @param InLocalPlayer The local player context
	 * @param InOutEditState The state to modify (can disable, hide, or kill the setting)
	 */
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState) const
	{
	}

	/** 
	 * @brief Generates debugging text for this edit condition
	 * @return A string representing the condition's state for debugging
	 */
	virtual FString ToString() const { return TEXT(""); }
};

/**
 * @brief Edit condition that checks the platform-specific frame pacing mode
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingEditCondition_FramePacingMode final : public FGameplaySettingEditCondition
{
public:
	/**
	 * @brief Constructs a frame pacing mode edit condition
	 * @param InDesiredMode The mode to check against
	 * @param InMatchMode Whether to enable or disable based on the match
	 */
	FGameplaySettingEditCondition_FramePacingMode(EGameplayFramePacingMode InDesiredMode, EGameplayFramePacingEditCondition InMatchMode = EGameplayFramePacingEditCondition::EnableIf)
		: DesiredMode(InDesiredMode)
		, MatchMode(InMatchMode)
	{}

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState) const override
	{
		const EGameplayFramePacingMode ActualMode = UGameplayPlatformSpecificRenderingSettings::Get()->FramePacingMode;

		const bool bMatches = (ActualMode == DesiredMode);
		const bool bMatchesAreBad = (MatchMode == EGameplayFramePacingEditCondition::DisableIf);

		if (bMatches == bMatchesAreBad)
		{
			InOutEditState.Kill(FString::Printf(TEXT("Frame pacing mode %d didn't match requirement %d"), (int32)ActualMode, (int32)DesiredMode));
		}
	}
private:
	/** TheDesired frame pacing mode */
	EGameplayFramePacingMode DesiredMode;
	/** Match behavior (EnableIf or DisableIf) */
	EGameplayFramePacingEditCondition MatchMode;
};

/**
 * @brief Edit condition that checks if the platform supports granular video quality settings
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingEditCondition_VideoQuality final : public FGameplaySettingEditCondition
{
public:
	/**
	 * @brief Constructs a video quality edit condition
	 * @param InDisableString The reason string to provide if granular settings are unsupported
	 */
	FGameplaySettingEditCondition_VideoQuality(const FString& InDisableString)
		: DisableString(InDisableString)
	{
	}

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState) const override
	{
		if (!UGameplayPlatformSpecificRenderingSettings::Get()->bSupportsGranularVideoQualitySettings)
		{
			InOutEditState.Kill(DisableString);
		}
	}

	virtual void SettingChanged(const ULocalPlayer* InLocalPlayer, UGameplaySetting* InSetting, EGameplaySettingChangeReason InReason) const override
	{
		if (const IGameplayCommonSettingsInterface* SettingsAccessor = Cast<IGameplayCommonSettingsInterface>(InLocalPlayer))
		{
			UGameplaySettingsLocal* LocalSettings = SettingsAccessor->GetLocalSettings();
			if (LocalSettings)
			{
				LocalSettings->ApplyScalabilitySettings();
			}
		}
	}

private:
	/** Reason string for disabling granular quality */
	FString DisableString;
};

/**
 * @brief Edit condition that checks if latency stats are supported on the current platform
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingEditCondition_LatencyStatsSupported final : public FGameplaySettingEditCondition
{
public:
	FGameplaySettingEditCondition_LatencyStatsSupported() = default;

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState) const override
	{
		if (!UGameplaySettingsLocal::DoesPlatformSupportLatencyTrackingStats())
		{
			InOutEditState.Disable(LOCTEXT("PlatformDoesNotSupportLatencyStates", "Latency performance stats are not supported on this device"));
		}
	}
};

/**
 * @brief Edit condition that checks if latency stats are currently enabled
 * 
 * Listens for changes in the latency stat indicator settings and updates the setting state.
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingEditCondition_LatencyStatsCurrentlyEnabled final : public FGameplaySettingEditCondition
{
public:
	FGameplaySettingEditCondition_LatencyStatsCurrentlyEnabled() = default;

	virtual ~FGameplaySettingEditCondition_LatencyStatsCurrentlyEnabled() override
	{
		if (!SettingChangedDelegate.IsValid()) return;

		UGameplaySettingsLocal* Settings = UGameplaySettingsLocal::Get();
		if (!Settings) return;

		Settings->OnLatencyStatIndicatorSettingsChangedEvent().Remove(SettingChangedDelegate);
	}

private:
	virtual void Initialize(const ULocalPlayer* InLocalPlayer) override
	{
		// Bind to an event for when the settings are updated so that we can broadcast that we need
		// to be re-evaluated
		UGameplaySettingsLocal* Settings = UGameplaySettingsLocal::Get();
		if (!Settings) return;

		SettingChangedDelegate = Settings->OnLatencyStatIndicatorSettingsChangedEvent().AddSP(this->AsShared(), 
			&FGameplaySettingEditCondition_LatencyStatsCurrentlyEnabled::BroadcastEditConditionChanged);
	}

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState) const override
	{
		const UGameplaySettingsLocal* Settings = UGameplaySettingsLocal::Get();
		if (Settings && !Settings->GetEnableLatencyTrackingStats())
		{
			InOutEditState.Disable(LOCTEXT("LatencyMarkerRequireStatsEnabled", "Latency Tracking Stats must be enabled to use this."));
		}
	}

private:
	/** Delegate handle for the settings change event */
	FDelegateHandle SettingChangedDelegate;
};

/**
 * @brief Edit condition that checks if latency markers are supported on the platform
 * 
 * Also optionally checks if NVIDIA hardware is present if configured via CVar.
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingEditCondition_LatencyMarkersSupported final : public FGameplaySettingEditCondition
{
public:
	FGameplaySettingEditCondition_LatencyMarkersSupported() = default;

	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState) const override
	{
		if (!UGameplaySettingsLocal::DoesPlatformSupportLatencyMarkers())
		{
			InOutEditState.Disable(LOCTEXT("PlatformDoesNotSupportLatencyMarkers", "Latency markers are not supported on this device"));
		}

		// Only use Reflex/latency stats on NVIDIA devices if the cvar is set.
		if (CVarLatencyMarkersRequireNVIDIA.GetValueOnAnyThread() && !IsRHIDeviceNVIDIA())
		{
			InOutEditState.Disable(LOCTEXT("InputLatencyMarkersRequiresNVIDIA", "Latency markers only work on NVIDIA devices."));
		}
	}
};

#undef LOCTEXT_NAMESPACE
