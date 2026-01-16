// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingValueScalar.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingValueScalar)

UGameplaySettingValueScalar::UGameplaySettingValueScalar(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameplaySettingValueScalar::SetValueNormalized(double NormalizedValue)
{
	SetValue(FMath::GetMappedRangeValueClamped(TRange<double>(0, 1), GetSourceRange(), NormalizedValue));
}

double UGameplaySettingValueScalar::GetValueNormalized() const
{
	return FMath::GetMappedRangeValueClamped(GetSourceRange(), TRange<double>(0, 1), GetValue());
}
