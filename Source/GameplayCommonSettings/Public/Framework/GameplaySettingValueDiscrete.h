// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplaySettingValue.h"
#include "Misc/GameplayCommonPerformanceSettings.h"
#include "GameplaySettingValueDiscrete.generated.h"

namespace EWindowMode { enum Type : int; }

class UObject;
struct FScreenResolutionRHI;

/**
 * @brief Base class for settings that have a discrete set of options (e.g., ComboBox, Rotator)
 * 
 * UGameplaySettingValueDiscrete represents a setting that can only take one value
 * from a predefined list of options. It provides an interface for getting and
 * setting the current selection by index.
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscrete : public UGameplaySettingValue
{
	GENERATED_BODY()

public:
	UGameplaySettingValueDiscrete(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ~Begin UGameplaySetting interface
	virtual FString GetAnalyticsValue() const override;
	// ~End of UGameplaySetting interface
	
	/**
	 * @brief Sets the current value by its index in the options list
	 * @param Index The index of the option to select
	 */
	virtual void SetDiscreteOptionByIndex(int32 Index) PURE_VIRTUAL(UGameplaySettingValueDiscrete::SetDiscreteOptionByIndex, );

	/**
	 * @brief Gets the index of the currently selected option
	 * @return The current option index, or INDEX_NONE if no selection
	 */
	UFUNCTION(BlueprintCallable, Category="Discrete")
	virtual int32 GetDiscreteOptionIndex() const PURE_VIRTUAL(UGameplaySettingValueDiscrete::GetDiscreteOptionIndex, return INDEX_NONE;);

	/**
	 * @brief Gets the index of the default option
	 * @return The default option index, or INDEX_NONE if no default
	 */
	UFUNCTION(BlueprintCallable, Category="Discrete")
	virtual int32 GetDiscreteOptionDefaultIndex() const { return INDEX_NONE; }

	/**
	 * @brief Gets the list of available text descriptions for all options
	 * @return An array of localized text for each option
	 */
	UFUNCTION(BlueprintCallable, Category="Discrete")
	virtual TArray<FText> GetDiscreteOptions() const PURE_VIRTUAL(UGameplaySettingValueDiscrete::GetDiscreteOptions, return TArray<FText>(););
};

/**
 * @brief Specialization for screen resolution settings
 * 
 * Handles the complexities of available resolutions across different window modes
 * (Fullscreen, Windowed, Windowed Fullscreen).
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscrete_Resolution final : public UGameplaySettingValueDiscrete
{
	GENERATED_BODY()

public:
	UGameplaySettingValueDiscrete_Resolution(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ~Begin UGameplaySettingValue interface
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;
	virtual void OnInitialized() override;
	virtual void OnDependencyChanged() override;
	// ~End of UGameplaySettingValue interface

	// ~Begin UGameplaySettingValueDiscrete interface
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	virtual int32 GetDiscreteOptionIndex() const override;
	virtual TArray<FText> GetDiscreteOptions() const override;
	// ~End of UGameplaySettingValueDiscrete interface

protected:
	/** @brief Populates the resolution lists from the system */
	void InitializeResolutions();

	/** @brief Checks if a resolution is suitable for fullscreen mode */
	bool ShouldAllowFullscreenResolution(const FScreenResolutionRHI& SrcScreenRes, int32 FilterThreshold) const;

	/** @brief Generates a list of standard windowed resolutions */
	static void GetStandardWindowResolutions(const FIntPoint& MinResolution, const FIntPoint& MaxResolution, float MinAspectRatio, TArray<FIntPoint>& OutResolutions);
	
	/** @brief Filters the resolution lists based on the current window mode and display */
	void SelectAppropriateResolutions();

	/** @brief Finds the index of a specific resolution in the current list */
	int32 FindIndexOfDisplayResolution(const FIntPoint& InPoint) const;
	
	/** @brief Finds the index of a resolution, ensuring a valid index is returned */
	int32 FindIndexOfDisplayResolutionForceValid(const FIntPoint& InPoint) const;
	
	/** @brief Finds the index of the resolution closest to the provided dimensions */
	int32 FindClosestResolutionIndex(const FIntPoint& Resolution) const;

	/** The last window mode used to filter resolutions */
	TOptional<EWindowMode::Type> LastWindowMode;

	/** Internal entry for a single resolution option */
	struct FScreenResolutionEntry
	{
		uint32 Width = 0;
		uint32 Height = 0;
		uint32 RefreshRate = 0;
		FText OverrideText;

		/** @brief Gets the dimensions as an FIntPoint */
		FIntPoint GetResolution() const { return FIntPoint(Width, Height); }

		/** @brief Gets the formatted display text (e.g., "1920x1080 (60Hz)") */
		FText GetDisplayText() const;
	};

	/** Current combined array of resolutions based on the window mode */
	TArray<TSharedPtr<FScreenResolutionEntry>> Resolutions;

	/** Cached list of all available fullscreen resolutions */
	TArray<TSharedPtr<FScreenResolutionEntry>> ResolutionsFullscreen;

	/** Cached list of all available windowed fullscreen resolutions */
	TArray<TSharedPtr<FScreenResolutionEntry>> ResolutionsWindowedFullscreen;

	/** Cached list of all available windowed resolutions */
	TArray<TSharedPtr<FScreenResolutionEntry>> ResolutionsWindowed;
};

/**
 * @brief Specialization for overall graphics quality settings
 * 
 * Provides a simplified way to set all scalability settings through a single index.
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscrete_OverallQuality final : public UGameplaySettingValueDiscrete
{
	GENERATED_BODY()

public:
	UGameplaySettingValueDiscrete_OverallQuality(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// ~Begin UGameplaySettingValue interface
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;
	virtual void OnInitialized() override;
	// ~End of UGameplaySettingValue interface

	// ~Begin UGameplaySettingValueDiscrete interface
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	virtual int32 GetDiscreteOptionIndex() const override;
	virtual TArray<FText> GetDiscreteOptions() const override;
	// ~End of UGameplaySettingValueDiscrete interface

protected:
	/** @brief Gets the index used for "Custom" when settings are mixed */
	int32 GetCustomOptionIndex() const;

	/** @brief Resolves the current scalablity level into a quality index */
	int32 GetOverallQualityLevel() const;

protected:
	/** Static list of quality level texts (Low, Medium, High, etc.) */
	TArray<FText> Options;

	/** Quality level texts including the "Custom" entry */
	TArray<FText> OptionsWithCustom;
};

/**
 * @brief Specialization for performance statistic display settings
 * 
 * Allows users to choose how individual performance stats are displayed
 * (Hidden, Text, Graph, etc.).
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscrete_PerfStat final : public UGameplaySettingValueDiscrete
{
	GENERATED_BODY()

public:
	UGameplaySettingValueDiscrete_PerfStat();

	/**
	 * @brief Sets which performance stat this setting controls
	 * @param InStat The performance stat to associate with this setting
	 */
	void SetStat(EGameplayDisplayablePerformanceStat InStat);

	// ~Begin UGameplaySettingValue interface
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;
	virtual void OnInitialized() override;
	// ~End of UGameplaySettingValue interface

	// ~Begin UGameplaySettingValueDiscrete interface
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	virtual int32 GetDiscreteOptionIndex() const override;
	virtual TArray<FText> GetDiscreteOptions() const override;
	// ~End of UGameplaySettingValueDiscrete interface

protected:
	/** @brief Internal helper to add a display mode option */
	void AddMode(FText&& Label, EGameplayPerformanceStatDisplayMode Mode);

protected:
	/** The list of localized display mode names */
	TArray<FText> Options;

	/** The corresponding display mode enums */
	TArray<EGameplayPerformanceStatDisplayMode> DisplayModes;

	/** The stat currently being controlled */
	EGameplayDisplayablePerformanceStat StatToDisplay;
	
	/** The initial display mode for restoration */
	EGameplayPerformanceStatDisplayMode InitialMode;
};

/**
 * @brief Specialization for language and culture settings
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscrete_Language final : public UGameplaySettingValueDiscrete
{
	GENERATED_BODY()

public:
	UGameplaySettingValueDiscrete_Language();

	// ~Begin UGameplaySettingValue interface
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;
	virtual void OnInitialized() override;
	virtual void OnApply() override;
	// ~End of UGameplaySettingValue interface

	// ~Begin UGameplaySettingValueDiscrete interface
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	virtual int32 GetDiscreteOptionIndex() const override;
	virtual TArray<FText> GetDiscreteOptions() const override;
	// ~End of UGameplaySettingValueDiscrete interface

protected:
	/** List of culture identifiers (e.g., "en-US", "pt-BR") available in the game */
	TArray<FString> AvailableCultureNames;
};


