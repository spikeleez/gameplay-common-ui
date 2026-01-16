// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Engine/PlatformSettings.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettingsBackedByCVars.h"
#include "Misc/EnumRange.h"
#include "GameplayCommonPerformanceSettings.generated.h"

/**
 * @brief Specifies how a performance statistic should be displayed in the UI
 */
UENUM(BlueprintType)
enum class EGameplayPerformanceStatDisplayMode : uint8
{
	/** Don't show this stat */
	Hidden					UMETA(DisplayName = "Hidden"),

	/** Show this stat in text form only */
	TextOnly				UMETA(DisplayName = "Text Only"),

	/** Show this stat in graph form only */
	GraphOnly				UMETA(DisplayName = "Graph Only"),

	/** Show this stat as both text and graph */
	TextAndGraph			UMETA(DisplayName = "Text & Graph"),

	/** New display mode should go above here */
	MAX						UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EGameplayPerformanceStatDisplayMode, EGameplayPerformanceStatDisplayMode::MAX);

/**
 * @brief Redefines various performance stats that can be tracked and displayed
 */
UENUM(BlueprintType)
enum class EGameplayDisplayablePerformanceStat : uint8
{
	/** Stat fps (in Hz) */
	ClientFPS				UMETA(DisplayName = "FPS (Client)"),

	/** Server tick rate (in Hz) */
	ServerFPS				UMETA(DisplayName = "FPS (Server)"),

	/** Idle time spent waiting for vsync or frame rate limit (in seconds) */
	IdleTime				UMETA(DisplayName = "Idle Time"),

	/** Stat unit overall (in seconds) */
	FrameTime_Total			UMETA(DisplayName = "Frame Time (Total)"),

	/** Stat unit (game thread, in seconds) */
	FrameTime_GameThread	UMETA (DisplayName = "Frame Time (Game Thread)"),

	/** Stat unit (render thread, in seconds) */
	FrameTime_RenderThread	UMETA(DisplayName = "Frame Time (Render Thread)"),

	/** Stat unit (RHI thread, in seconds) */
	FrameTime_RHIThread		UMETA(DisplayName = "Frame Time (RHI Thread)"),

	/** Stat unit (inferred GPU time, in seconds) */
	FrameTime_GPU			UMETA(DisplayName = "Frame Time (GPU)"),

	/** Network ping (in ms) */
	Ping					UMETA(DisplayName = "Ping (ms)"),

	/** The incoming packet loss percentage (%) */
	PacketLoss_Incoming		UMETA(DisplayName = "Packet Loss (Incoming)"),

	/** The outgoing packet loss percentage (%) */
	PacketLoss_Outgoing		UMETA(DisplayName = "Packet Loss (Outgoing)"),

	/** The number of packets received in the last second */
	PacketRate_Incoming		UMETA(DisplayName = "Packet Rate (Incoming)"),

	/** The number of packets sent in the past seconds */
	PacketRate_Outgoing     UMETA(DisplayName = "Packet Rate (Outgoing)"),

	/** The avg. size (in bytes) of packets received */
	PacketSize_Incoming		UMETA(DisplayName = "Packet Size (Incoming)"),

	/** The avg. size (in bytes) of packets sent */
	PacketSize_Outgoing		UMETA(DisplayName = "Packet Size (Outgoing)"),

	/** The total latency in MS of the game */
	Latency_Total			UMETA(DisplayName = "Latency (Total)"),

	/** Game simulation start to driver submission end */
	Latency_Game			UMETA(DisplayName = "Latency (Game)"),

	/** OS render queue start to GPU render end */
	Latency_Render			UMETA(DisplayName = "Latency (Render)"),

	/** New stats should go above here */
	MAX						UMETA(Hidden)
};
ENUM_RANGE_BY_COUNT(EGameplayDisplayablePerformanceStat, EGameplayDisplayablePerformanceStat::MAX);

/**
 * @brief Specifies how frame pacing and graphics settings are exposed for a platform
 */
UENUM()
enum class EGameplayFramePacingMode : uint8
{
	/** Manual frame rate limits, user is allowed to choose whether to lock to vsync */
	DesktopStyle,

	/** Limits handled by choosing present intervals driven by device profiles */
	ConsoleStyle,

	/** Limits handled by a user-facing choice of frame rate from among ones allowed by device profiles */
	MobileStyle
};

/**
 * @brief Describes one platform-specific device profile variant that the user can choose
 */
USTRUCT()
struct FGameplayQualityDeviceProfileVariant
{
	GENERATED_BODY()

	/** @brief The display name for this device profile variant (visible in the options screen) */
	UPROPERTY(EditAnywhere, Category = "Profile")
	FText DisplayName;

	/** @brief The suffix to append to the base device profile name for the current platform */
	UPROPERTY(EditAnywhere, Category = "Profile")
	FString DeviceProfileSuffix;

	/** 
	 * @brief The minimum required refresh rate to enable this mode
	 * 
	 * If the device's current refresh rate is lower than this, the mode won't be available.
	 */
	UPROPERTY(EditAnywhere, Category = "Profile")
	int32 MinRefreshRate = 0;
};

/**
 * @brief Describes a set of performance stats predicated on platform traits
 */
USTRUCT()
struct FGameplayPerformanceStatGroup
{
	GENERATED_BODY()

	/** 
	 * @brief A query on platform traits to determine if this stat group is visible
	 */
	UPROPERTY(EditAnywhere, Category = "Stat Group", meta=(Categories = "Input,Platform.Trait"))
	FGameplayTagQuery VisibilityQuery;

	/** @brief The set of stats to allow if the query passes */
	UPROPERTY(EditAnywhere, Category = "Stat Group")
	TSet<EGameplayDisplayablePerformanceStat> AllowedStats;
};

/**
 * @brief Platform-specific rendering and performance settings
 */
UCLASS(Config=Game, DefaultConfig, meta = (DisplayName = "Gameplay Common: Platform Rendering Settings"))
class GAMEPLAYCOMMONSETTINGS_API UGameplayPlatformSpecificRenderingSettings : public UPlatformSettings
{
	GENERATED_BODY()

public:
	UGameplayPlatformSpecificRenderingSettings();

	/**
	 * @brief Helper method to get the performance settings object
	 * @return The singleton instance of the settings
	 */
	static const UGameplayPlatformSpecificRenderingSettings* Get();

public:
	/**
	 * @brief The default variant suffix to append to the device profile
	 * 
	 * Should typically be a member of UserFacingDeviceProfileOptions.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Device Profiles")
	FString DefaultDeviceProfileSuffix;

	/**
	 * @brief The list of device profile variations to allow users to choose from
	 *
	 * These should be sorted from slowest to fastest by target frame rate.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Device Profiles")
	TArray<FGameplayQualityDeviceProfileVariant> UserFacingDeviceProfileOptions;

	/** @brief Does the platform support granular video quality settings? */
	UPROPERTY(Config, EditAnywhere, Category = "Video Settings")
	bool bSupportsGranularVideoQualitySettings = true;

	/** @brief Does the platform support running the automatic quality benchmark? */
	UPROPERTY(Config, EditAnywhere, Category = "Video Settings")
	bool bSupportsAutomaticVideoQualityBenchmark = true;

	/** @brief Specifies how frame pacing is controlled on this platform */
	UPROPERTY(Config, EditAnywhere, Category = "Video Settings")
	EGameplayFramePacingMode FramePacingMode = EGameplayFramePacingMode::DesktopStyle;

	/**
	 * @brief Potential frame rates to display for mobile platforms
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Video Settings", meta = (EditCondition = "FramePacingMode==EGameplayFramePacingMode::MobileStyle", ForceUnits = "Hz"))
	TArray<int32> MobileFrameRateLimits;
};

/**
 * @brief Project-wide performance and scalability settings
 */
UCLASS(Config=Game, DefaultConfig, meta=(DisplayName = "Gameplay Common: Performance Settings"))
class GAMEPLAYCOMMONSETTINGS_API UGameplayCommonPerformanceSettings : public UDeveloperSettingsBackedByCVars
{
	GENERATED_BODY()

public:
	UGameplayCommonPerformanceSettings();

public:
	/**
	 * @brief The list of frame rates to allow users to choose from on desktop platforms
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Performance", meta = (ForceUnits = "Hz"))
	TArray<int32> DesktopFrameRateLimits;

	/** @brief The list of performance stats that can be enabled in Options by the user */
	UPROPERTY(Config, EditAnywhere, Category = "Stats")
	TArray<FGameplayPerformanceStatGroup> UserFacingPerformanceStats;

private:
	/**
	 * @brief Helper to expose per-platform settings in the project settings UI
	 */
	UPROPERTY(EditAnywhere, Category = "Platform Specific")
	FPerPlatformSettings PerPlatformSettings;
};