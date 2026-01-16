// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingValueScalarDynamic.h"
#include "DataSource/GameplaySettingDataSource.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingValueScalarDynamic)

#define LOCTEXT_NAMESPACE "GameplayGameSettingValueScalarDynamic"

static FText PercentFormat = LOCTEXT("PercentFormat", "{0}%");

UGameplaySettingValueScalarDynamic::UGameplaySettingValueScalarDynamic(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FGameplaySettingValueScalarFormatFunction UGameplaySettingValueScalarDynamic::Raw([](double SourceValue, double NormalizedValue)
{
	return FText::AsNumber(SourceValue);
});

FGameplaySettingValueScalarFormatFunction UGameplaySettingValueScalarDynamic::RawOneDecimal([](double SourceValue, double NormalizedValue)
{
	FNumberFormattingOptions FormattingOptions;
	FormattingOptions.MinimumIntegralDigits = 1;
	FormattingOptions.MinimumFractionalDigits = 1;
	FormattingOptions.MaximumFractionalDigits = 1;
	
	return FText::AsNumber(SourceValue, &FormattingOptions);
});

FGameplaySettingValueScalarFormatFunction UGameplaySettingValueScalarDynamic::RawTwoDecimals([](double SourceValue, double NormalizedValue)
{
	FNumberFormattingOptions FormattingOptions;
	FormattingOptions.MinimumIntegralDigits = 1;
	FormattingOptions.MinimumFractionalDigits = 2;
	FormattingOptions.MaximumFractionalDigits = 2;
	
	return FText::AsNumber(SourceValue, &FormattingOptions);
});

FGameplaySettingValueScalarFormatFunction UGameplaySettingValueScalarDynamic::SourceAsInteger([](double SourceValue, double NormalizedValue)
{
	FNumberFormattingOptions FormattingOptions;
	FormattingOptions.MinimumIntegralDigits = 1;
	FormattingOptions.MinimumFractionalDigits = 0;
	FormattingOptions.MaximumFractionalDigits = 0;
	
	return FText::AsNumber(SourceValue, &FormattingOptions);
});

FGameplaySettingValueScalarFormatFunction UGameplaySettingValueScalarDynamic::ZeroToOnePercent([](double SourceValue, double NormalizedValue)
{
	return FText::Format(PercentFormat, (int32)FMath::RoundHalfFromZero(100.0 * NormalizedValue));
});

FGameplaySettingValueScalarFormatFunction UGameplaySettingValueScalarDynamic::ZeroToOnePercent_OneDecimal([](double SourceValue, double NormalizedValue)
{
	const FNumberFormattingOptions& FormattingOptions = GetOneDecimalFormattingOptions();
	const double NormalizedValueTo100_0 = FMath::RoundHalfFromZero(1000.0 * NormalizedValue);
	return FText::Format(PercentFormat, FText::AsNumber(NormalizedValueTo100_0 / 10.0, &FormattingOptions));
});

FGameplaySettingValueScalarFormatFunction UGameplaySettingValueScalarDynamic::SourceAsPercent1([](double SourceValue, double NormalizedValue)
{
	return FText::Format(PercentFormat, (int32)FMath::RoundHalfFromZero(100.0 * SourceValue));
});

FGameplaySettingValueScalarFormatFunction UGameplaySettingValueScalarDynamic::SourceAsPercent100([](double SourceValue, double NormalizedValue)
{
	return FText::Format(PercentFormat, (int32)FMath::RoundHalfFromZero(SourceValue));
});

const FNumberFormattingOptions& UGameplaySettingValueScalarDynamic::GetOneDecimalFormattingOptions()
{
	static FNumberFormattingOptions FormattingOptions;
	FormattingOptions.MinimumFractionalDigits = 1;
	FormattingOptions.MaximumFractionalDigits = 1;

	return FormattingOptions;
}

void UGameplaySettingValueScalarDynamic::Startup()
{
	Getter->Startup(LocalPlayer, FSimpleDelegate::CreateUObject(this, &ThisClass::OnDataSourcesReady));
}

void UGameplaySettingValueScalarDynamic::StoreInitial()
{
	InitialValue = GetValue();
}

void UGameplaySettingValueScalarDynamic::ResetToDefault()
{
	if (DefaultValue.IsSet())
	{
		SetValue(DefaultValue.GetValue(), EGameplaySettingChangeReason::ResetToDefault);
	}
}

void UGameplaySettingValueScalarDynamic::RestoreToInitial()
{
	SetValue(InitialValue, EGameplaySettingChangeReason::RestoreToInitial);
}

TOptional<double> UGameplaySettingValueScalarDynamic::GetDefaultValue() const
{
	return DefaultValue;
}

void UGameplaySettingValueScalarDynamic::SetValue(double Value, EGameplaySettingChangeReason Reason)
{
	Value = FMath::RoundHalfToZero(Value / SourceStep);
	Value = Value * SourceStep;

	if (Minimum.IsSet())
	{
		Value = FMath::Max(Minimum.GetValue(), Value);
	}

	if (Maximum.IsSet())
	{
		Value = FMath::Min(Maximum.GetValue(), Value);
	}

	const FString StringValue = LexToString(Value);
	Setter->SetValue(LocalPlayer, StringValue);

	NotifySettingChanged(Reason);
}

double UGameplaySettingValueScalarDynamic::GetValue() const
{
	const FString OutValue = Getter->GetValueAsString(LocalPlayer);

	double Value;
	LexFromString(Value, *OutValue);

	return Value;
}

TRange<double> UGameplaySettingValueScalarDynamic::GetSourceRange() const
{
	return SourceRange;
}

double UGameplaySettingValueScalarDynamic::GetSourceStep() const
{
	return SourceStep;
}

FText UGameplaySettingValueScalarDynamic::GetFormattedText() const
{
	const double SourceValue = GetValue();
	const double NormalizedValue = GetValueNormalized();

	return DisplayFormat(SourceValue, NormalizedValue);
}

void UGameplaySettingValueScalarDynamic::SetDynamicGetter(const TSharedRef<FGameplaySettingDataSource>& InGetter)
{
	Getter = InGetter;
}

void UGameplaySettingValueScalarDynamic::SetDynamicSetter(const TSharedRef<FGameplaySettingDataSource>& InSetter)
{
	Setter = InSetter;
}

void UGameplaySettingValueScalarDynamic::SetDefaultValue(double InValue)
{
	DefaultValue = InValue;
}

void UGameplaySettingValueScalarDynamic::SetDisplayFormat(const FGameplaySettingValueScalarFormatFunction& InDisplayFormat)
{
	DisplayFormat = InDisplayFormat;
}

void UGameplaySettingValueScalarDynamic::SetSourceRangeAndStep(const TRange<double>& InRange, double InSourceStep)
{
	SourceRange = InRange;
	SourceStep = InSourceStep;
}

void UGameplaySettingValueScalarDynamic::SetMinimumLimit(const TOptional<double>& InMinimum)
{
	Minimum = InMinimum;
}

void UGameplaySettingValueScalarDynamic::SetMaximumLimit(const TOptional<double>& InMaximum)
{
	Maximum = InMaximum;
}

void UGameplaySettingValueScalarDynamic::OnInitialized()
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(DisplayFormat, TEXT("%s: Has no DisplayFormat set.  Please call SetDisplayFormat."), *GetDevName().ToString());
#endif

#if !UE_BUILD_SHIPPING
	ensureAlways(Getter);
	ensureAlwaysMsgf(Getter->Resolve(LocalPlayer), TEXT("%s: %s did not resolve, are all functions and properties valid, and are they UFunctions/UProperties?"),
		*GetDevName().ToString(), *Getter->ToString());
	ensureAlways(Setter);
	ensureAlwaysMsgf(Setter->Resolve(LocalPlayer), TEXT("%s: %s did not resolve, are all functions and properties valid, and are they UFunctions/UProperties?"),
		*GetDevName().ToString(), *Setter->ToString());
#endif

	Super::OnInitialized();
}

void UGameplaySettingValueScalarDynamic::OnDataSourcesReady()
{
	StartupComplete();
}

#undef LOCTEXT_NAMESPACE