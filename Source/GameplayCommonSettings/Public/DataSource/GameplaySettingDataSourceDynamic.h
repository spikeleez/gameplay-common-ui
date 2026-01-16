// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplaySettingDataSource.h"
#include "PropertyPathHelpers.h"

class ULocalPlayer;

/**
 * @brief Data source that uses reflection (Property Paths) to access data
 * 
 * FGameplaySettingDataSourceDynamic allows settings to dynamically read and write
 * to any UObject property that can be reached via a property path from the LocalPlayer
 * or related objects.
 */
class GAMEPLAYCOMMONSETTINGS_API FGameplaySettingDataSourceDynamic : public FGameplaySettingDataSource
{
public:
	/**
	 * @brief Constructs a dynamic data source from a property path
	 * @param InDynamicPath Array of string segments forming the property path (e.g. {"PlayerController", "MyVariable"})
	 */
	FGameplaySettingDataSourceDynamic(const TArray<FString>& InDynamicPath);

	// ~Begin FGameplaySettingDataSource interface
	virtual bool Resolve(ULocalPlayer* InLocalPlayer) override;
	virtual FString GetValueAsString(ULocalPlayer* InLocalPlayer) const override;
	virtual void SetValue(ULocalPlayer* InLocalPlayer, const FString& Value) override;
	virtual FString ToString() const override;
	// ~End of FGameplaySettingDataSource interface

private:
	/** The cached reflection path to the target property */
	FCachedPropertyPath DynamicPath;
};


