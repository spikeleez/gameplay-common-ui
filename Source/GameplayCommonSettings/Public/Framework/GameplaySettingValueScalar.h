// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplaySettingValue.h"
#include "GameplaySettingValueScalar.generated.h"

/**
 * @brief Base class for settings representable as a numeric scalar value (e.g., Sliders)
 * 
 * UGameplaySettingValueScalar provides an interface for handling values within a 
 * specific range. It supports normalized values (0 to 1) for easy integration 
 * with UI widgets liked sliders, while maintaining the actual source value range internally.
 */
UCLASS(Abstract)
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueScalar : public UGameplaySettingValue
{
	GENERATED_BODY()

public:
	UGameplaySettingValueScalar(const FObjectInitializer& ObjectInitializer);

	/**
	 * @brief Sets the value using a normalized range (0 to 1)
	 * @param NormalizedValue The value to set, between 0 and 1
	 */
	void SetValueNormalized(double NormalizedValue);
	
	/**
	 * @brief Gets the current value mapped to a normalized range (0 to 1)
	 * @return The normalized value
	 */
	double GetValueNormalized() const;

	/**
	 * @brief Gets the default value mapped to a normalized range (0 to 1)
	 * @return The normalized default value, or unset if no default exists
	 */
	TOptional<double> GetDefaultValueNormalized() const
	{
		TOptional<double> DefaultValue = GetDefaultValue();
		if (DefaultValue.IsSet())
		{
			return FMath::GetMappedRangeValueClamped(GetSourceRange(), TRange<double>(0, 1), DefaultValue.GetValue());
		}
		return TOptional<double>();
	}

	/** @brief Gets the actual default value in the source range */
	virtual TOptional<double> GetDefaultValue() const PURE_VIRTUAL(UGameplaySettingValueScalar::GetDefaultValue, return TOptional<double>(););
	
	/**
	 * @brief Sets the actual value in the source range
	 * @param Value The value to set
	 * @param Reason The reason for the change
	 */
	virtual void SetValue(double Value, EGameplaySettingChangeReason Reason = EGameplaySettingChangeReason::Change) PURE_VIRTUAL(UGameplaySettingValueScalar::SetValue, );
	
	/** @brief Gets the actual current value in the source range */
	virtual double GetValue() const PURE_VIRTUAL(UGameplaySettingValueScalar::GetValue, return 0;);
	
	/** @brief Gets the valid range of values in the source space */
	virtual TRange<double> GetSourceRange() const PURE_VIRTUAL(UGameplaySettingValueScalar::GetSourceRange, return TRange<double>(););
	
	/** @brief Gets the minimum increment for value changes in the source space */
	virtual double GetSourceStep() const PURE_VIRTUAL(UGameplaySettingValueScalar::GetSourceStep, return 0.01;);
	
	/** @brief Gets the step size mapped to the normalized (0-1) space */
	double GetNormalizedStepSize() const
	{
		const TRange<double> SourceRange = GetSourceRange();
		return GetSourceStep() / FMath::Abs(SourceRange.GetUpperBoundValue() - SourceRange.GetLowerBoundValue());
	}

	/** @brief Gets the formatted localized text representation of the current value */
	virtual FText GetFormattedText() const PURE_VIRTUAL(UGameplaySettingValueScalar::GetFormattedText, return FText::GetEmpty(););

	// ~Begin UGameplaySetting interface
	/** @brief Gets the value string for analytics reporting */
	virtual FString GetAnalyticsValue() const override
	{
		return LexToString(GetValue());
	}
	// ~End of UGameplaySetting interface
};
