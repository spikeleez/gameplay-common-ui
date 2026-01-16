// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "Widgets/SLeafWidget.h"
#include "Misc/GameplayCommonPerformanceSettings.h"
#include "Subsystems/GameplayPerformanceStatSubsystem.h"
#include "GameplayPerfStatWidgetBase.generated.h"

/**
 * @brief Slate widget for rendering a latency/performance graph
 * 
 * SGameplayLatencyGraph is a low-level Slate widget designed to efficiently 
 * render a line graph of performance data (like frame times) using 
 * the engine's rendering context. It supports background color and line color customization.
 */
class SGameplayLatencyGraph : public SLeafWidget
{
public:
	/** Begin the arguments for this slate widget. */
	SLATE_BEGIN_ARGS(SGameplayLatencyGraph)
		: _DesiredSize(150, 50)
		, _MaxLatencyToGraph(33.0)
		, _LineColor(255, 255, 255, 255)
		, _BackgroundColor(0, 0, 0, 128)
	{
		_Clipping = EWidgetClipping::ClipToBounds;
	}

	/** @brief The expected size of the graph */
	SLATE_ARGUMENT(FVector2D, DesiredSize)

	/** @brief The maximum value representing the top of the Y-axis */
	SLATE_ARGUMENT(double, MaxLatencyToGraph)

	/** @brief The color of the graph line */
	SLATE_ARGUMENT(FColor, LineColor)

	/** @brief The background color of the graph area */
	SLATE_ARGUMENT(FColor, BackgroundColor)
	
	SLATE_END_ARGS()

	/** @brief Construct function needed for every widget */
	void Construct(const FArguments& InArgs);

	/** @brief Called with the elements to be drawn. */
	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyClippingRect, FSlateWindowElementList& OutDrawElements, 
		int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;

	/** @brief Graphs are volatile because they update every frame */
	virtual bool ComputeVolatility() const override { return true; }

	/** @brief Returns DesiredSize */
	virtual FVector2D ComputeDesiredSize(float LayoutScaleMultiplier) const override;

	/** @brief Updates the line color */
	void SetLineColor(const FColor& InColor)
	{
		LineColor = InColor;
	}

	/** @brief Updates the Y-axis maximum scale */
	void SetMaxYValue(const double InValue)
	{
		MaxYAxisOfGraph = InValue;
	}

	/** @brief Updates the background color */
	void SetBackgroundColor(const FColor& InColor)
	{
		BackgroundColor = InColor;
	}

	/** @brief Supplies the data cache to be rendered */
	void UpdateGraphData(const FGameplaySampledStatCache* StatData, const float InScaleFactor)
	{
		GraphData = StatData;
		ScaleFactor = InScaleFactor;
	}

private:
	/** @brief Internal render helper for graph line */
	void DrawTotalLatency(const FGeometry& AllottedGeometry, FSlateWindowElementList& OutDrawElements, int32 LayerId) const;

	/** The size of the graph to draw */
	FVector2D DesiredSize = { 150.0, 50.0 };

	/** Max Y value of the graph. The values drawn will be clamped to this */
	double MaxYAxisOfGraph = 33.0;

	/** Multiplier applied to stat values (e.g., to convert units) */
	float ScaleFactor = 1.0f;

	/** Color of the line to draw on the graph */
	FColor LineColor = FColor(255, 255, 255, 255);

	/** The background color to draw when drawing the graph */
	FColor BackgroundColor = FColor(0, 0, 0, 128);

	/** The cache of data that this graph widget needs to draw */
	const FGameplaySampledStatCache* GraphData = nullptr;
};

/**
 * @brief UMG Wrapper for SGameplayLatencyGraph
 * 
 * This class exposes the low-level Slate graph widget to UMG, making it 
 * usable within the Unreal Editor Designer.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta = (DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplayPerfStatGraph : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UGameplayPerfStatGraph(const FObjectInitializer& ObjectInitializer);

	/** @brief Proxy to SGameplayLatencyGraph::SetLineColor */
	void SetLineColor(const FColor& InColor);
	/** @brief Proxy to SGameplayLatencyGraph::SetMaxYValue */
	void SetMaxYValue(const float InValue);
	/** @brief Proxy to SGameplayLatencyGraph::SetBackgroundColor */
	void SetBackgroundColor(const FColor& InValue);
	/** @brief Proxy to SGameplayLatencyGraph::UpdateGraphData */
	void UpdateGraphData(const FGameplaySampledStatCache* StatData, const float ScaleFactor);

protected:
	/** The actual Slate handle managed by this UWidget */
	TSharedPtr<SGameplayLatencyGraph> SlateLatencyGraph;

protected:
	// ~Begin UWidget interface
	virtual TSharedRef<SWidget> RebuildWidget() override;
	virtual void ReleaseSlateResources(bool bReleaseChildren) override;
	// ~End of UWidget interface
};

/**
 * @brief Base class for widgets displaying a single performance statistic
 * 
 * UGameplayPerfStatWidgetBase handles common functionality for displaying
 * performance metrics like FPS, Ping, or Packet Loss. It supports showing a
 * live graph if a UGameplayPerfStatGraph is provided in the widget hierarchy.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplayPerfStatWidgetBase : public UCommonUserWidget
{
	GENERATED_BODY()

public:
	UGameplayPerfStatWidgetBase(const FObjectInitializer& ObjectInitializer);

	// ~Begin UUserWidget interface
	virtual void NativeConstruct() override;
	// ~End of UUserWidget interface

	/**
	 * @brief Gets the performance stat type this widget is configured to display
	 * @return The performance stat enum value
	 */
	UFUNCTION(BlueprintPure, Category = "Performance Stats")
	EGameplayDisplayablePerformanceStat GetStatToDisplay() const
	{
		return StatToDisplay;
	}

	/**
	 * @brief Polls the latest value for the associated stat
	 * @return The current raw (unscaled) statistic value
	 */
	UFUNCTION(BlueprintPure, Category = "Performance Stats")
	double FetchStatValue();

	/**
	 * @brief Updates the associated graph with new data
	 * @param ScaleFactor Multiplier to adjust stat visibility (e.g. for conversion)
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Stats")
	void UpdateGraphData(const float ScaleFactor = 1.0f);

protected:
	/** @brief Helper to retrieve the global performance stat subsystem */
	UGameplayPerformanceStatSubsystem* GetStatSubsystem();

protected:
	/**
	 * @brief An optional stat graph widget to display this stat's value over time.
	 * 
	 * Named 'PerfStatGraph' in the widget designer to enable automatic binding.
	 */
	UPROPERTY(BlueprintReadWrite, Category="Designer", meta=(BindWidget, OptionalWidget=true))
	TObjectPtr<UGameplayPerfStatGraph> PerfStatGraph;

	/** Cached pointer to the stat subsystem */
	UPROPERTY(Transient)
	TObjectPtr<UGameplayPerformanceStatSubsystem> CachedStatSubsystem;

	/** Defined line color for the graph if present */
	UPROPERTY(EditAnywhere, Category = "Display")
	FColor GraphLineColor = FColor(255, 255, 255, 255);

	/** Defined background color for the graph if present */
	UPROPERTY(EditAnywhere, Category = "Display")
	FColor GraphBackgroundColor = FColor(0, 0, 0, 128);

	/**
	 * @brief The max value of the Y axis to clamp the graph to.
	 */
	UPROPERTY(EditAnywhere, Category = "Display")
	double GraphMaxYValue = 33.0;

	/** The performance statistic this widget is intended to track */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Display")
	EGameplayDisplayablePerformanceStat StatToDisplay;
};