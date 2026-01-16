// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplaySettingValueScalar.h"
#include "GameplaySettingValueScalarDynamic.generated.h"

struct FNumberFormattingOptions;

class FGameplaySettingDataSource;
class UObject;

/** @brief Type definition for functions used to format scalar values into localized text */
typedef TFunction<FText(double SourceValue, double NormalizedValue)> FGameplaySettingValueScalarFormatFunction;

/**
 * @brief Represents a scalar setting that fetches and stores its value dynamically
 * 
 * UGameplaySettingValueScalarDynamic interfaces with external data (like C++ properties
 * or CVars) using data sources. It provides extensive support for formatting
 * numeric values into text and supports restricting the user-interactive range
 * within the actual source data range.
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueScalarDynamic : public UGameplaySettingValueScalar
{
	GENERATED_BODY()

public:
	/** @brief Standard formatting: Raw value (e.g., "0.1234") */
	static FGameplaySettingValueScalarFormatFunction Raw;

	/** @brief Standard formatting: One decimal place (e.g., "0.1") */
	static FGameplaySettingValueScalarFormatFunction RawOneDecimal;

	/** @brief Standard formatting: Two decimal places (e.g., "0.12") */
	static FGameplaySettingValueScalarFormatFunction RawTwoDecimals;

	/** @brief Standard formatting: 0-1 mapped to 0-100% (e.g., "12%") */
	static FGameplaySettingValueScalarFormatFunction ZeroToOnePercent;

	/** @brief Standard formatting: 0-1 mapped to 0-100% with one decimal (e.g., "12.3%") */
	static FGameplaySettingValueScalarFormatFunction ZeroToOnePercent_OneDecimal;

	/** @brief Standard formatting: Source value as percentage base 1 (Multiplied by 100) */
	static FGameplaySettingValueScalarFormatFunction SourceAsPercent1;

	/** @brief Standard formatting: Source value as percentage base 100 (Directly as %) */
	static FGameplaySettingValueScalarFormatFunction SourceAsPercent100;

	/** @brief Standard formatting: Source value as a rounded integer */
	static FGameplaySettingValueScalarFormatFunction SourceAsInteger;

public:
	UGameplaySettingValueScalarDynamic(const FObjectInitializer& ObjectInitializer);

	// ~Begin UGameplaySettingValue interface
	virtual void Startup() override;
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;
	// ~End of UGameplaySettingValue interface

	// ~Begin UGameplaySettingValueScalar interface
	virtual TOptional<double> GetDefaultValue() const override;
	virtual void SetValue(double Value, EGameplaySettingChangeReason Reason = EGameplaySettingChangeReason::Change) override;
	virtual double GetValue() const override;
	virtual TRange<double> GetSourceRange() const override;
	virtual double GetSourceStep() const override;
	virtual FText GetFormattedText() const override;
	// ~End of UGameplaySettingValueScalar interface

	/** @brief Sets the data source for reading values */
	void SetDynamicGetter(const TSharedRef<FGameplaySettingDataSource>& InGetter);

	/** @brief Sets the data source for writing values */
	void SetDynamicSetter(const TSharedRef<FGameplaySettingDataSource>& InSetter);

	/** @brief Sets the default fallback value */
	void SetDefaultValue(double InValue);

	/** @brief Sets the function used to format the numeric value for UI display */
	void SetDisplayFormat(const FGameplaySettingValueScalarFormatFunction& InDisplayFormat);
	
	/** 
	 * @brief Sets the source range and step size 
	 * @param InRange The full range of possible values
	 * @param InSourceStep The minimum increment for value changes
	 */
	void SetSourceRangeAndStep(const TRange<double>& InRange, double InSourceStep);

	/**
	 * @brief Sets an artificial minimum limit for user interaction
	 * 
	 * The slider will still show the full range, but the user cannot select a value
	 * below this limit.
	 * @param InMinimum The minimum limit, or unset to remove limit
	 */
	void SetMinimumLimit(const TOptional<double>& InMinimum);
	
	/**
	 * @brief Sets an artificial maximum limit for user interaction
	 * 
	 * The slider will still show the full range, but the user cannot select a value
	 * above this limit.
	 * @param InMaximum The maximum limit, or unset to remove limit
	 */
	void SetMaximumLimit(const TOptional<double>& InMaximum);

protected:
	/** The data source for reading values */
	TSharedPtr<FGameplaySettingDataSource> Getter;

	/** The data source for writing values */
	TSharedPtr<FGameplaySettingDataSource> Setter;

	/** The default value */
	TOptional<double> DefaultValue;

	/** The value at the time the settings were opened */
	double InitialValue = 0;

	/** The range of possible values */
	TRange<double> SourceRange = TRange<double>(0, 1);

	/** The increment step size */
	double SourceStep = 0.01;

	/** User-interaction minimum limit override */
	TOptional<double> Minimum;
	
	/** User-interaction maximum limit override */
	TOptional<double> Maximum;

	/** The function used for localized text formatting */
	FGameplaySettingValueScalarFormatFunction DisplayFormat;
	
protected:
	// ~Begin UGameplaySettingValue interface
	virtual void OnInitialized() override;
	// ~End of UGameplaySettingValue interface

	/** @brief Callback for when all data sources are ready */
	void OnDataSourcesReady();

private:
	/** @brief Internal helper for numeric formatting options */
	static const FNumberFormattingOptions& GetOneDecimalFormattingOptions();
};


