// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "ChartCreation.h"
#include "Misc/GameplayCommonPerformanceSettings.h"
#include "Algo/MinElement.h"
#include "Algo/MaxElement.h"
#include "GameplayPerformanceStatSubsystem.generated.h"

class FSubsystemCollectionBase;
class UGameplayPerformanceStatSubsystem;
class UObject;
struct FFrame;

/**
 * @brief Stores a buffer of performance stat samples and provides statistical analysis
 * 
 * FGameplaySampledStatCache maintains a ring buffer of statistical samples
 * and provides utilities to calculate minimum, maximum, and average values
 * over the sample period.
 * 
 * This class is useful for tracking performance metrics over time and
 * generating charts or visualizations of performance data.
 * 
 * @note Uses a ring buffer for efficient memory usage
 */
class FGameplaySampledStatCache
{
public:
	/**
	 * @brief Constructs a sampled stat cache with the specified buffer size
	 * @param InSampleSize Number of samples to store in the buffer (default: 125)
	 */
	FGameplaySampledStatCache(const int32 InSampleSize = 125)
		: SampleSize(InSampleSize)
	{
		check(InSampleSize > 0);

		Samples.Empty();
		Samples.AddZeroed(SampleSize);
	}

	/**
	 * @brief Records a new sample in the buffer
	 * 
	 * Adds a sample to the ring buffer at the current index position.
	 * Automatically wraps around when the buffer is full.
	 * 
	 * @param Sample The value to record
	 */
	void RecordSample(const double Sample)
	{
		// A simple little ring buffer for storing the samples over time.
		Samples[CurrentSampleIndex] = Sample;

		CurrentSampleIndex++;
		if (CurrentSampleIndex >= Samples.Num())
		{
			CurrentSampleIndex = 0u;
		}
	}

	/**
	 * @brief Gets the current cached stat at the current index
	 * @return The stat value at the current sample index
	 */
	double GetCurrentCachedStat() const
	{
		return Samples[CurrentSampleIndex];
	}

	/**
	 * @brief Gets the most recently recorded stat
	 * @return The stat value from the previous sample
	 */
	double GetLastCachedStat() const
	{
		int32 LastIndex = CurrentSampleIndex - 1;
		if (LastIndex < 0)
		{
			LastIndex = Samples.Num() - 1;
		}

		return Samples[LastIndex];
	}

	/**
	 * @brief Iterates through all samples, starting with the most recent
	 * 
	 * @param Func Function to call for each sample
	 * 
	 * Example usage:
	 * @code
	 * Cache.ForEachCurrentSample([](double Stat) {
	 *     UE_LOG(LogTemp, Log, TEXT("Sample: %f"), Stat);
	 * });
	 * @endcode
	 */
	void ForEachCurrentSample(const TFunctionRef<void(const double Stat)> Func) const
	{
		int32 Index = CurrentSampleIndex;
		for (int32 i = 0; i < SampleSize; i++)
		{
			Func(Samples[Index]);

			Index++;
			if (Index == SampleSize)
			{
				Index = 0;
			}
		}
	}

	/**
	 * @brief Gets the total number of samples stored in the buffer
	 * @return The sample buffer size
	 */
	int32 GetSampleSize() const
	{
		return SampleSize;
	}

	/**
	 * @brief Calculates the average of all samples in the buffer
	 * @return The average value across all samples
	 */
	double GetAverage() const
	{
		double Sum = 0.0;
		ForEachCurrentSample([&Sum](const double Stat)
		{
			Sum += Stat;
		});

		// Return the average stat value.
		return Sum / static_cast<double>(SampleSize);
	}

	/**
	 * @brief Gets the minimum value among all samples
	 * @return The minimum sample value
	 */
	double GetMin() const
	{
		return *Algo::MinElement(Samples);
	}

	/**
	 * @brief Gets the maximum value among all samples
	 * @return The maximum sample value
	 */
	double GetMax() const
	{
		return *Algo::MaxElement(Samples);
	}

private:
	/** The number of samples to store */
	const int32 SampleSize = 125;
	
	/** Current position in the ring buffer */
	int32 CurrentSampleIndex = 0;

	/** Array storing all sample values */
	TArray<double> Samples;
};

/**
 * @brief Observer that caches performance stats for each frame
 * 
 * FGameplayPerformanceStatCache implements IPerformanceDataConsumer to
 * receive and cache performance data from the engine's charting system.
 * It maintains a cache of sampled stat data for various performance metrics.
 * 
 * @see IPerformanceDataConsumer, FGameplaySampledStatCache
 */
struct FGameplayPerformanceStatCache : IPerformanceDataConsumer
{
public:
	/**
	 * @brief Constructs the performance stat cache
	 * @param InSubsystem The owning subsystem
	 */
	FGameplayPerformanceStatCache(UGameplayPerformanceStatSubsystem* InSubsystem)
		: MySubsystem(InSubsystem)
	{
	}

	// ~IPerformanceDataConsumer interface
	virtual void StartCharting() override;
	virtual void ProcessFrame(const FFrameData& FrameData) override;
	virtual void StopCharting() override;
	// ~End of IPerformanceDataConsumer interface

	/**
	 * @brief Returns the latest cached value for the given stat type
	 * @param Stat The performance stat type to query
	 * @return The most recent value for the specified stat
	 */
	double GetCachedStat(EGameplayDisplayablePerformanceStat Stat) const;

	/**
	 * @brief Returns a pointer to the cache for the given stat type
	 * 
	 * This can be used to get the min/max/average of this stat, the latest stat,
	 * and iterate all the samples. Useful for generating UI like an FPS chart over time.
	 * 
	 * @param Stat The performance stat type to query
	 * @return Pointer to the sampled stat cache, or nullptr if not found
	 */
	 const FGameplaySampledStatCache* GetCachedStatData(const EGameplayDisplayablePerformanceStat Stat) const;

protected:
	/**
	 * @brief Records a stat sample
	 * @param Stat The stat type
	 * @param Value The value to record
	 */
	void RecordStat(const EGameplayDisplayablePerformanceStat Stat, const double Value);

protected:
	/** The owning subsystem */
	UGameplayPerformanceStatSubsystem* MySubsystem;

	/** Caches the sampled data for each of the performance stats currently available */
	TMap<EGameplayDisplayablePerformanceStat, FGameplaySampledStatCache> PerfStatCache;
};

/**
 * @brief Subsystem that provides access to performance stats for display purposes
 * 
 * UGameplayPerformanceStatSubsystem is a game instance subsystem that tracks
 * and caches performance statistics such as FPS, frame time, and other metrics.
 * 
 * This subsystem integrates with the engine's performance charting system and
 * provides a Blueprint-accessible interface for querying performance data.
 * 
 * Use this subsystem to display performance metrics in your UI, create
 * performance graphs, or implement dynamic quality adjustments based on
 * current performance.
 * 
 * @see UGameInstanceSubsystem, FGameplayPerformanceStatCache
 */
UCLASS(meta=(DisplayName="Gameplay Performance Stat Subsystem"))
class UGameplayPerformanceStatSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UGameplayPerformanceStatSubsystem();

	// ~Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	// ~End of USubsystem interface

	/**
	 * @brief Gets the most recent cached value for a performance stat
	 * 
	 * @param Stat The performance stat type to query
	 * @return The current value of the specified stat
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Stats")
	double GetCachedStat(EGameplayDisplayablePerformanceStat Stat) const;

	/**
	 * @brief Gets detailed cache data for a performance stat
	 * 
	 * Returns the full sampled cache which includes min/max/average calculations
	 * and access to historical samples.
	 * 
	 * @param Stat The performance stat type to query
	 * @return Pointer to the sampled stat cache, or nullptr if not found
	 */
	const FGameplaySampledStatCache* GetCachedStatData(const EGameplayDisplayablePerformanceStat Stat) const;

protected:
	/** The performance stat tracker and cache */
	TSharedPtr<FGameplayPerformanceStatCache> Tracker;
};