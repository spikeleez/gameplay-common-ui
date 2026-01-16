// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "AudioDeviceNotificationSubsystem.h"
#include "AudioMixerBlueprintLibrary.h"
#include "GameplaySettingValueDiscrete.h"
#include "DataSource/GameplaySettingDataSource.h"
#include "GameplaySettingValueDiscreteDynamic.generated.h"

/**
 * @brief Represents a discrete setting that fetches and stores its options dynamically
 * 
 * UGameplaySettingValueDiscreteDynamic uses data sources (Getter/Setter) to 
 * interface with external data (like C++ properties or CVars). Options are 
 * added as string/text pairs, allowing for flexible mapping between internal 
 * values and user-facing text.
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscreteDynamic : public UGameplaySettingValueDiscrete
{
	GENERATED_BODY()

public:
	UGameplaySettingValueDiscreteDynamic(const FObjectInitializer& ObjectInitializer);

	// ~Begin UGameplaySettingValue interface
	virtual void Startup() override;
	virtual void StoreInitial() override;
	virtual void ResetToDefault() override;
	virtual void RestoreToInitial() override;
	// ~End of UGameplaySettingValue interface

	// ~Begin UGameplaySettingValueDiscrete interface
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	virtual int32 GetDiscreteOptionIndex() const override;
	virtual int32 GetDiscreteOptionDefaultIndex() const override;
	virtual TArray<FText> GetDiscreteOptions() const override;
	// ~End of UGameplaySettingValueDiscrete interface

	/** @brief Sets the data source used to read the current value */
	void SetDynamicGetter(const TSharedRef<FGameplaySettingDataSource>& InGetter);

	/** @brief Sets the data source used to write the current value */
	void SetDynamicSetter(const TSharedRef<FGameplaySettingDataSource>& InSetter);

	/** @brief Sets the default fallback value for this setting */
	void SetDefaultValueFromString(FString InOptionValue);

	/** @brief Adds a new option to the dynamic list */
	void AddDynamicOption(FString InOptionValue, FText InOptionText);

	/** @brief Removes an option from the dynamic list */
	void RemoveDynamicOption(FString InOptionValue);

	/** @brief Checks if a specific value is present in the options list */
	bool HasDynamicOption(const FString& InOptionValue);

	/** @brief Gets the list of internal option values (as strings) */
	const TArray<FString>& GetDynamicOptions();

	/** @brief Reads the raw value from the getter as a string */
	FString GetValueAsString() const;

	/** @brief Writes a raw string value through the setter */
	void SetValueFromString(const FString& InStringValue);

protected:
	/** The data source for reading values */
	TSharedPtr<FGameplaySettingDataSource> Getter;

	/** The data source for writing values */
	TSharedPtr<FGameplaySettingDataSource> Setter;

	/** The default value as a string */
	TOptional<FString> DefaultValue;

	/** The value at the time the settings were opened */
	FString InitialValue;

	/** Internal list of option values (mapped to data source) */
	TArray<FString> OptionValues;

	/** Internal list of localized display texts */
	TArray<FText> OptionDisplayTexts;
	
protected:
	/** @brief Internal helper to set value with a change reason */
	void SetValueFromString(const FString& InStringValue, EGameplaySettingChangeReason Reason);

	// ~Begin UGameplaySettingValue interface
	virtual void OnInitialized() override;
	// ~End of UGameplaySettingValue interface

	/** @brief Callback for when all data sources are ready */
	void OnDataSourceReady();

	/** @brief Internal helper for value comparison */
	bool AreOptionsEqual(const FString& InOptionA, const FString& InOptionB) const;
};

/**
 * @brief Dynamic discrete setting for boolean values (True/False)
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscreteDynamic_Bool : public UGameplaySettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	UGameplaySettingValueDiscreteDynamic_Bool(const FObjectInitializer& ObjectInitializer);

	/** @brief Sets the default boolean value */
	void SetDefaultValue(bool Value);

	/** @brief Sets the display text for the 'True' state */
	void SetTrueText(const FText& Value);
	
	/** @brief Sets the display text for the 'False' state */
	void SetFalseText(const FText& Value);

#if !UE_BUILD_SHIPPING
	/** @brief Sets the display text for the 'True' state from a string (non-shipping) */
	void SetTrueText(const FString& Value) { SetTrueText(FText::FromString(Value)); }

	/** @brief Sets the display text for the 'False' state from a string (non-shipping) */
	void SetFalseText(const FString& Value) { SetFalseText(FText::FromString(Value)); }
#endif
};

/**
 * @brief Dynamic discrete setting for numeric values (Integers, Floats)
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscreteDynamic_Number : public UGameplaySettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	UGameplaySettingValueDiscreteDynamic_Number(const FObjectInitializer& ObjectInitializer);

	/** @brief Sets the default numeric value */
	template<typename NumberType>
	void SetDefaultValue(NumberType InValue)
	{
		SetDefaultValueFromString(LexToString(InValue));
	}

	/** @brief Adds a numeric option with display text */
	template<typename NumberType>
	void AddOption(NumberType InValue, const FText& InOptionText)
	{
		AddDynamicOption(LexToString(InValue), InOptionText);
	}

	/** @brief Gets the current value cast to the desired numeric type */
	template<typename NumberType>
	NumberType GetValue() const
	{
		const FString ValueString = GetValueAsString();

		NumberType OutValue;
		LexFromString(OutValue, *ValueString);

		return OutValue;
	}

	/** @brief Sets the current value from a numeric type */
	template<typename NumberType>
	void SetValue(NumberType InValue)
	{
		SetValueFromString(LexToString(InValue));
	}

protected:
	// ~Begin UGameplaySettingValue interface
	virtual void OnInitialized() override;
	// ~End of UGameplaySettingValue interface
};

/**
 * @brief Dynamic discrete setting for enum values
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscreteDynamic_Enum : public UGameplaySettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	UGameplaySettingValueDiscreteDynamic_Enum(const FObjectInitializer& ObjectInitializer);

	/** @brief Sets the default enum value */
	template<typename EnumType>
	void SetDefaultValue(EnumType InEnumValue)
	{
		const FString StringValue = StaticEnum<EnumType>()->GetNameStringByValue((int64)InEnumValue);
		SetDefaultValueFromString(StringValue);
	}

	/** @brief Adds an enum option with associated display text */
	template<typename EnumType>
	void AddOption(EnumType InEnumValue, const FText& InOptionText)
	{
		const FString StringValue = StaticEnum<EnumType>()->GetNameStringByValue((int64)InEnumValue);
		AddDynamicOption(StringValue, InOptionText);
	}

	/** @brief Gets the current value as the specified enum type */
	template<typename EnumType>
	EnumType GetValue() const
	{
		const FString Value = GetValueAsString();
		return (EnumType)StaticEnum<EnumType>()->GetValueByNameString(Value);
	}

	/** @brief Sets the current value using an enum */
	template<typename EnumType>
	void SetValue(EnumType InEnumValue)
	{
		const FString StringValue = StaticEnum<EnumType>()->GetNameStringByValue((int64)InEnumValue);
		SetValueFromString(StringValue);
	}

protected:
	// ~Begin UGameplaySettingValue interface
	virtual void OnInitialized() override;
	// ~End of UGameplaySettingValue interface
};

/**
 * @brief Dynamic discrete setting for color values
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscreteDynamic_Color : public UGameplaySettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	UGameplaySettingValueDiscreteDynamic_Color(const FObjectInitializer& ObjectInitializer);

	/** @brief Sets the default linear color */
	void  SetDefaultValue(FLinearColor InColor)
	{
		SetDefaultValueFromString(InColor.ToString());
	}

	/** @brief Adds a color option (display text is automatically generated as a hex string) */
	void AddColorOption(FLinearColor InColor)
	{
		const FColor SRGBColor = InColor.ToFColor(true);
		AddDynamicOption(InColor.ToString(), FText::FromString(FString::Printf(TEXT("#%02X%02X%02X"), SRGBColor.R, SRGBColor.G, SRGBColor.B)));
	}

	/** @brief Gets the current value as a FLinearColor */
	FLinearColor GetValue() const
	{
		const FString Value = GetValueAsString();

		FLinearColor ColorValue;
		const bool bSuccess = ColorValue.InitFromString(Value);
		ensure(bSuccess);

		return ColorValue;
	}

	/** @brief Sets the current value from a FLinearColor */
	void SetValue(FLinearColor InColor)
	{
		SetValueFromString(InColor.ToString());
	}
};

/**
 * @brief Dynamic discrete setting for 2D vectors
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscreteDynamic_Vector2D : public UGameplaySettingValueDiscreteDynamic
{
	GENERATED_BODY()

public:
	UGameplaySettingValueDiscreteDynamic_Vector2D(const FObjectInitializer& ObjectInitializer);

	/** @brief Sets the default 2D vector value */
	void SetDefaultValue(const FVector2D& Value)
	{
		SetDefaultValueFromString(Value.ToString());
	}

	/** @brief Gets the current value as a FVector2D */
	FVector2D GetValue() const
	{
		FVector2D ValueVector;
		ValueVector.InitFromString(GetValueAsString());
		return ValueVector;
	}

	/** @brief Sets the current value from a FVector2D */
	void SetValue(const FVector2D& Value)
	{
		SetValueFromString(Value.ToString());
	}
};

/**
 * @brief Dynamic discrete setting for Audio output devices
 */
UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingValueDiscreteDynamic_AudioOutputDevice : public UGameplaySettingValueDiscreteDynamic
{
	GENERATED_BODY()
	
public:
	virtual ~UGameplaySettingValueDiscreteDynamic_AudioOutputDevice() override = default;
	
	virtual void SetDiscreteOptionByIndex(int32 Index) override;
	
	UFUNCTION()
	void OnAudioOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices);
	
	UFUNCTION()
	void OnCompletedDeviceSwap(const FSwapAudioOutputResult& SwapResult);
	
	UFUNCTION()
	void DeviceAddedOrRemoved(FString DeviceId);
	
	UFUNCTION()
	void DefaultDeviceChanged(EAudioDeviceChangedRole InRole, FString DeviceId);
	
protected:
	virtual void OnInitialized() override;
	
protected:
	TArray<FAudioOutputDeviceInfo> OutputDevices;
	
	FString CurrentDeviceId;
	FString SystemDefaultDeviceId;
	
	int32 LastKnownGoodIndex = 0;
	
	bool bRequestDefault = false;
	
	FOnAudioOutputDevicesObtained DevicesObtainedCallback;
	FOnCompletedDeviceSwap DevicesSwappedCallback;
};


