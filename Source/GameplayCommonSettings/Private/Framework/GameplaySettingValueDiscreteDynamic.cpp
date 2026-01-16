// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingValueDiscreteDynamic.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingValueDiscreteDynamic)

#define LOCTEXT_NAMESPACE "SettingValueDiscreteDynamic"

// =========================================================
// UGameplaySettingValueDiscreteDynamic
// =========================================================
UGameplaySettingValueDiscreteDynamic::UGameplaySettingValueDiscreteDynamic(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UGameplaySettingValueDiscreteDynamic::Startup()
{
	// Should I also do something with Setter?
	check(Getter);
	Getter->Startup(LocalPlayer, FSimpleDelegate::CreateUObject(this, &ThisClass::OnDataSourceReady));
}

void UGameplaySettingValueDiscreteDynamic::StoreInitial()
{
	InitialValue = GetValueAsString();
}

void UGameplaySettingValueDiscreteDynamic::ResetToDefault()
{
	if (DefaultValue.IsSet())
	{
		SetValueFromString(DefaultValue.GetValue(), EGameplaySettingChangeReason::ResetToDefault);
	}
}

void UGameplaySettingValueDiscreteDynamic::RestoreToInitial()
{
	SetValueFromString(InitialValue, EGameplaySettingChangeReason::RestoreToInitial);
}

void UGameplaySettingValueDiscreteDynamic::SetDiscreteOptionByIndex(int32 Index)
{
	if (ensure(OptionValues.IsValidIndex(Index)))
	{
		SetValueFromString(OptionValues[Index]);
	}
}

int32 UGameplaySettingValueDiscreteDynamic::GetDiscreteOptionIndex() const
{
	const FString CurrentValue = GetValueAsString();
	const int32 Index = OptionValues.IndexOfByPredicate([this, CurrentValue](const FString& InOptionValue)
	{
		return AreOptionsEqual(CurrentValue, InOptionValue);
	});

	// If we can't find the correct index, send the default index.
	if (Index == INDEX_NONE)
	{
		return GetDiscreteOptionDefaultIndex();
	}

	return Index;
}

int32 UGameplaySettingValueDiscreteDynamic::GetDiscreteOptionDefaultIndex() const
{
	if (DefaultValue.IsSet())
	{
		return OptionValues.IndexOfByPredicate([this](const FString& InOptionValue)
		{
			return AreOptionsEqual(DefaultValue.GetValue(), InOptionValue);
		});
	}

	return INDEX_NONE;
}

TArray<FText> UGameplaySettingValueDiscreteDynamic::GetDiscreteOptions() const
{
	const TArray<FString>& DisabledOptions = GetEditState().GetDisabledOptions();

	if (DisabledOptions.Num() > 0)
	{
		TArray<FText> AllowedOptions;
		for (int32 OptionIndex = 0; OptionIndex < DisabledOptions.Num(); ++OptionIndex)
		{
			if (!DisabledOptions.Contains(OptionValues[OptionIndex]))
			{
				AllowedOptions.Add(OptionDisplayTexts[OptionIndex]);
			}
		}

		return AllowedOptions;
	}

	return OptionDisplayTexts;
}

void UGameplaySettingValueDiscreteDynamic::SetDynamicGetter(const TSharedRef<FGameplaySettingDataSource>& InGetter)
{
	Getter = InGetter;
}

void UGameplaySettingValueDiscreteDynamic::SetDynamicSetter(const TSharedRef<FGameplaySettingDataSource>& InSetter)
{
	Setter = InSetter;
}

void UGameplaySettingValueDiscreteDynamic::SetDefaultValueFromString(FString InOptionValue)
{
	DefaultValue = InOptionValue;
}

void UGameplaySettingValueDiscreteDynamic::AddDynamicOption(FString InOptionValue, FText InOptionText)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!OptionValues.Contains(InOptionValue), TEXT("You already added this option InOptionValue: %s InOptionText %s."),
		*InOptionValue, *InOptionText.ToString());
#endif

	OptionValues.Add(InOptionValue);
	OptionDisplayTexts.Add(InOptionText);
}

void UGameplaySettingValueDiscreteDynamic::RemoveDynamicOption(FString InOptionValue)
{
	const int32 Index = OptionValues.IndexOfByKey(InOptionValue);
	if (Index != INDEX_NONE)
	{
		OptionValues.RemoveAt(Index);
		OptionDisplayTexts.RemoveAt(Index);
	}
}

const TArray<FString>& UGameplaySettingValueDiscreteDynamic::GetDynamicOptions()
{
	return OptionValues;
}

bool UGameplaySettingValueDiscreteDynamic::HasDynamicOption(const FString& InOptionValue)
{
	return OptionValues.Contains(InOptionValue);
}

FString UGameplaySettingValueDiscreteDynamic::GetValueAsString() const
{
	return Getter->GetValueAsString(LocalPlayer);
}

void UGameplaySettingValueDiscreteDynamic::SetValueFromString(const FString& InStringValue)
{
	SetValueFromString(InStringValue, EGameplaySettingChangeReason::Change);
}

void UGameplaySettingValueDiscreteDynamic::SetValueFromString(const FString& InStringValue, EGameplaySettingChangeReason Reason)
{
	check(Setter);
	Setter->SetValue(LocalPlayer, InStringValue);

	NotifySettingChanged(Reason);
}

void UGameplaySettingValueDiscreteDynamic::OnInitialized()
{
#if !UE_BUILD_SHIPPING
	ensureAlways(Getter);
	
	ensureAlwaysMsgf(Getter->Resolve(LocalPlayer),
		TEXT("%s: %s did not resolve, are all functions and properties valid, and are they UFunctions/UProperties? Does the getter function have no parameters?"),
		*GetDevName().ToString(), *Getter->ToString());
	ensureAlways(Setter);
	
	ensureAlwaysMsgf(Setter->Resolve(LocalPlayer),
		TEXT("%s: %s did not resolve, are all functions and properties valid, and are they UFunctions/UProperties? Does the setting function have exactly one parameter?"),
		*GetDevName().ToString(), *Setter->ToString());
#endif
	
	Super::OnInitialized();
}

void UGameplaySettingValueDiscreteDynamic::OnDataSourceReady()
{
	StartupComplete();
}

bool UGameplaySettingValueDiscreteDynamic::AreOptionsEqual(const FString& InOptionA, const FString& InOptionB) const
{
	return InOptionA == InOptionB;	
}

// =========================================================
// UGameplaySettingValueDiscreteDynamic_Bool
// =========================================================
UGameplaySettingValueDiscreteDynamic_Bool::UGameplaySettingValueDiscreteDynamic_Bool(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	AddDynamicOption(TEXT("false"), LOCTEXT("OFF", "OFF"));
	AddDynamicOption(TEXT("true"), LOCTEXT("ON", "ON"));
}

void UGameplaySettingValueDiscreteDynamic_Bool::SetDefaultValue(bool Value)
{
	DefaultValue = LexToString(Value);
}

void UGameplaySettingValueDiscreteDynamic_Bool::SetTrueText(const FText& Value)
{
	// We remove and then re-add it, so that by changing the true/false text you can also control the order they appear.
	RemoveDynamicOption(TEXT("true"));
	AddDynamicOption(TEXT("true"), Value);
}

void UGameplaySettingValueDiscreteDynamic_Bool::SetFalseText(const FText& Value)
{
	// We remove and then re-add it, so that by changing the true/false text you can also control the order they appear.
	RemoveDynamicOption(TEXT("false"));
	AddDynamicOption(TEXT("false"), Value);
}

// =========================================================
// UGameplaySettingValueDiscreteDynamic_Number
// =========================================================
UGameplaySettingValueDiscreteDynamic_Number::UGameplaySettingValueDiscreteDynamic_Number(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameplaySettingValueDiscreteDynamic_Number::OnInitialized()
{
	Super::OnInitialized();

	ensure(OptionValues.Num() > 0);
}

// =========================================================
// UGameplaySettingValueDiscreteDynamic_Enum
// =========================================================
UGameplaySettingValueDiscreteDynamic_Enum::UGameplaySettingValueDiscreteDynamic_Enum(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UGameplaySettingValueDiscreteDynamic_Enum::OnInitialized()
{
	Super::OnInitialized();

	ensure(OptionValues.Num() > 0);
}

// =========================================================
// UGameplaySettingValueDiscreteDynamic_Color
// =========================================================
UGameplaySettingValueDiscreteDynamic_Color::UGameplaySettingValueDiscreteDynamic_Color(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// =========================================================
// UGameplaySettingValueDiscreteDynamic_Vector2D
// =========================================================
UGameplaySettingValueDiscreteDynamic_Vector2D::UGameplaySettingValueDiscreteDynamic_Vector2D(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

// =========================================================
// UGameplaySettingValueDiscreteDynamic_AudioOutputDevice
// =========================================================
void UGameplaySettingValueDiscreteDynamic_AudioOutputDevice::OnInitialized()
{
	Super::OnInitialized();
	
	DevicesObtainedCallback.BindUFunction(this, FName("OnAudioOutputDevicesObtained"));
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedDeviceSwap"));

	if (UAudioDeviceNotificationSubsystem* AudioDeviceNotifSubsystem = UAudioDeviceNotificationSubsystem::Get())
	{
		AudioDeviceNotifSubsystem->DeviceAddedNative.AddUObject(this, &UGameplaySettingValueDiscreteDynamic_AudioOutputDevice::DeviceAddedOrRemoved);
		AudioDeviceNotifSubsystem->DeviceRemovedNative.AddUObject(this, &UGameplaySettingValueDiscreteDynamic_AudioOutputDevice::DeviceAddedOrRemoved);
		//AudioDeviceNotifSubsystem->DeviceSwitchedNative.AddUObject(this, &ULyraSettingValueDiscreteDynamic_AudioOutputDevice::DeviceSwitched);
		AudioDeviceNotifSubsystem->DefaultRenderDeviceChangedNative.AddUObject(this, &UGameplaySettingValueDiscreteDynamic_AudioOutputDevice::DefaultDeviceChanged);
	}

	UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(this, DevicesObtainedCallback);
}

void UGameplaySettingValueDiscreteDynamic_AudioOutputDevice::OnAudioOutputDevicesObtained(const TArray<FAudioOutputDeviceInfo>& AvailableDevices)
{
	int32 NewSize = AvailableDevices.Num();
	OutputDevices.Reset(NewSize++);
	OutputDevices.Append(AvailableDevices);

	OptionValues.Reset(NewSize);
	OptionDisplayTexts.Reset(NewSize);

	// Placeholder - needs to be first option so we can format the default device string later
	AddDynamicOption(TEXT(""), FText::GetEmpty());
	FString SystemDefaultDeviceName;

	for (const FAudioOutputDeviceInfo& DeviceInfo : OutputDevices)
	{
		if (!DeviceInfo.DeviceId.IsEmpty() && !DeviceInfo.Name.IsEmpty())
		{
			// System Default 
			if (DeviceInfo.bIsSystemDefault)
			{
				SystemDefaultDeviceId = DeviceInfo.DeviceId;
				SystemDefaultDeviceName = DeviceInfo.Name;
			}

			// Current Device
			if (DeviceInfo.bIsCurrentDevice)
			{
				CurrentDeviceId = DeviceInfo.DeviceId;
			}

			// Add the menu option
			AddDynamicOption(DeviceInfo.DeviceId, FText::FromString(DeviceInfo.Name));
		}
	}

	OptionDisplayTexts[0] = FText::Format(LOCTEXT("DefaultAudioOutputDevice", "Default Output - {0}"), FText::FromString(SystemDefaultDeviceName));
	SetDefaultValueFromString(TEXT(""));
	RefreshEditableState();

	//LastKnownGoodIndex = GetDiscreteOptionDefaultIndex();
	//SetDiscreteOptionByIndex(GetDiscreteOptionIndex());
}

void UGameplaySettingValueDiscreteDynamic_AudioOutputDevice::OnCompletedDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
	//if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
	//{
	//	UE_LOG(LogTemp, VeryVerbose, TEXT("AudioOutputDevice failure! Resetting to: %s"), *(OptionDisplayTexts[LastKnownGoodIndex].ToString()));
	//	if (OptionValues.Num() < LastKnownGoodIndex && SwapResult.RequestedDeviceId != OptionValues[LastKnownGoodIndex])
	//	{
	//		SetDiscreteOptionByIndex(LastKnownGoodIndex);
	//	}

	//	// Remove the invalid device
	//	if (SwapResult.RequestedDeviceId != SystemDefaultDeviceId)
	//	{
	//		OutputDevices.RemoveAll([&SwapResult](FAudioOutputDeviceInfo& Device)
	//		{
	//			return Device.DeviceId == SwapResult.RequestedDeviceId;
	//		});

	//		RemoveDynamicOption(SwapResult.RequestedDeviceId);
	//		RefreshEditableState();
	//	}
	//}
}

void UGameplaySettingValueDiscreteDynamic_AudioOutputDevice::DeviceAddedOrRemoved(FString DeviceId)
{
	UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(this, DevicesObtainedCallback);
}

void UGameplaySettingValueDiscreteDynamic_AudioOutputDevice::DefaultDeviceChanged(EAudioDeviceChangedRole InRole, FString DeviceId)
{
	UAudioMixerBlueprintLibrary::GetAvailableAudioOutputDevices(this, DevicesObtainedCallback);
}

void UGameplaySettingValueDiscreteDynamic_AudioOutputDevice::SetDiscreteOptionByIndex(int32 Index)
{
	Super::SetDiscreteOptionByIndex(Index);
	//UE_LOG(LogTemp, VeryVerbose, TEXT("AudioOutputDevice set to %s - %s"), *(OptionDisplayTexts[Index].ToString()), *OptionValues[Index]);
	//bRequestDefault = false;

	//FString RequestedAudioDeviceId = GetValueAsString();

	//// Grab the correct deviceId if the user has selected default
	//const int32 DefaultOptionIndex = GetDiscreteOptionDefaultIndex();
	//if (Index == DefaultOptionIndex)
	//{
	//	RequestedAudioDeviceId = SystemDefaultDeviceId;
	//}

	//// Only swap if the requested deviceId is different than our current
	//if (RequestedAudioDeviceId == CurrentDeviceId)
	//{
	//	LastKnownGoodIndex = Index;
	//	UE_LOG(LogTemp, VeryVerbose, TEXT("AudioOutputDevice (Not Swapping) - LKG set to index :%d"), LastKnownGoodIndex);
	//}
	//else
	//{
	//	bRequestDefault = (Index == DefaultOptionIndex);
	//	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(LocalPlayer, RequestedAudioDeviceId, DevicesSwappedCallback);
	//	UE_LOG(LogTemp, VeryVerbose, TEXT("AudioOutputDevice requesting %s"), *RequestedAudioDeviceId);
	//}
}

#undef LOCTEXT_NAMESPACE
