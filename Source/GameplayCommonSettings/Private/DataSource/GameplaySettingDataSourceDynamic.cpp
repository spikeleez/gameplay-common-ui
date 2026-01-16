// Copyright Spike Plugins 2026. All Rights Reserved.

#include "DataSource/GameplaySettingDataSourceDynamic.h"
#include "Engine/LocalPlayer.h"

FGameplaySettingDataSourceDynamic::FGameplaySettingDataSourceDynamic(const TArray<FString>& InDynamicPath)
	: DynamicPath(InDynamicPath)
{
}

bool FGameplaySettingDataSourceDynamic::Resolve(ULocalPlayer* InLocalPlayer)
{
	return DynamicPath.Resolve(InLocalPlayer);
}

FString FGameplaySettingDataSourceDynamic::GetValueAsString(ULocalPlayer* InLocalPlayer) const
{
	FString OutStringValue;

	const bool bSuccess = PropertyPathHelpers::GetPropertyValueAsString(InLocalPlayer, DynamicPath, OutStringValue);
	ensure(bSuccess);

	return OutStringValue;
}

void FGameplaySettingDataSourceDynamic::SetValue(ULocalPlayer* InLocalPlayer, const FString& Value)
{
	const bool bSuccess = PropertyPathHelpers::SetPropertyValueFromString(InLocalPlayer, DynamicPath, Value);
	ensure(bSuccess);
}

FString FGameplaySettingDataSourceDynamic::ToString() const
{
	return DynamicPath.ToString();
}
