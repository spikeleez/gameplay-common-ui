// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayGameLayout.h"
#include "CommonUISettings.h"
#include "CommonUIVisibilitySubsystem.h"
#include "Misc/GameplayCommonTags.h"
#include "Misc/GameplayCommonUILibrary.h"
#include "ICommonUIModule.h"
#include "GameFramework/InputDeviceSubsystem.h"
#include "GameFramework/InputSettings.h"
#include "Input/CommonUIInputTypes.h"
#include "Widgets/GameplayControllerDisconnectedScreen.h"

DEFINE_LOG_CATEGORY_STATIC(LogGameplayGameLayout, Log, All);

UGameplayGameLayout::UGameplayGameLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, SpawnedControllerDisconnectedScreen(nullptr)
{
	// By default, only primarily controller platforms require a disconnect screen. 
	PlatformRequiresControllerDisconnectScreen.AddTag(GameplayCommonTags::Traits_PrimaryController);
}

void UGameplayGameLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	RegisterUIActionBinding(FBindUIActionArgs(FUIActionTag::ConvertChecked(GameplayCommonTags::Action_Escape), 
		false, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleEscapeAction)));
	
	// If we can display a controller disconnect screen, then listen for the controller state change delegates
	if (ShouldPlatformDisplayControllerDisconnectScreen())
	{
		// Bind to when input device connections change
		const IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
		DeviceMapper.GetOnInputDeviceConnectionChange().AddUObject(this, &ThisClass::HandleInputDeviceConnectionChanged);
		DeviceMapper.GetOnInputDevicePairingChange().AddUObject(this, &ThisClass::HandleInputDevicePairingChanged);
	}
}

void UGameplayGameLayout::NativeDestruct()
{
	Super::NativeDestruct();
	
	// Remove bindings to input device connection changing
	const IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	DeviceMapper.GetOnInputDeviceConnectionChange().RemoveAll(this);
	DeviceMapper.GetOnInputDevicePairingChange().RemoveAll(this);
	
	if (RequestProcessControllerStateHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(RequestProcessControllerStateHandle);
		RequestProcessControllerStateHandle.Reset();
	}
}

void UGameplayGameLayout::HandleEscapeAction()
{
	if (ensure(!EscapeMenuClass.IsNull()))
	{
		UGameplayCommonUILibrary::PushStreamedActivatableWidgetForClass(GetOwningPlayer(), GameplayCommonTags::Layer_Menu, EscapeMenuClass);
	}
}

void UGameplayGameLayout::HandleInputDeviceConnectionChanged(EInputDeviceConnectionState NewConnectionState, FPlatformUserId PlatformUserId,
	FInputDeviceId InputDeviceId)
{
	const FPlatformUserId OwningLocalPlayerId = GetOwningLocalPlayer()->GetPlatformUserId();
	ensure(OwningLocalPlayerId.IsValid());
	
	// This device connection change happened to a different player, ignore it for us.
	if (PlatformUserId != OwningLocalPlayerId)
	{
		return;
	}
	
	NotifyControllerStateChangeForDisconnectScreen();
}

void UGameplayGameLayout::HandleInputDevicePairingChanged(FInputDeviceId InputDeviceId, FPlatformUserId NewUserPlatformId,
	FPlatformUserId OldUserPlatformId)
{
	const FPlatformUserId OwningLocalPlayerId = GetOwningLocalPlayer()->GetPlatformUserId();
	ensure(OwningLocalPlayerId.IsValid());
	
	// If this pairing change was related to our local player, notify of a change.
	if (NewUserPlatformId == OwningLocalPlayerId || OldUserPlatformId == OwningLocalPlayerId)
	{
		NotifyControllerStateChangeForDisconnectScreen();
	}
}

bool UGameplayGameLayout::ShouldPlatformDisplayControllerDisconnectScreen() const
{
	// We only want this menu on primarily controller platforms
	bool bHasAllRequiredTags = ICommonUIModule::GetSettings().GetPlatformTraits().HasAll(PlatformRequiresControllerDisconnectScreen);
	
	// Check the tags that we may be emulating in the editor too
#if WITH_EDITOR
	const FGameplayTagContainer& PlatformEmulationTags = UCommonUIVisibilitySubsystem::Get(GetOwningLocalPlayer())->GetVisibilityTags();
	bHasAllRequiredTags |= PlatformEmulationTags.HasAll(PlatformRequiresControllerDisconnectScreen);
#endif
	
	return bHasAllRequiredTags;
}

void UGameplayGameLayout::NotifyControllerStateChangeForDisconnectScreen()
{
	// We should only ever get here if we have bound to the controller state change delegates
	ensure(ShouldPlatformDisplayControllerDisconnectScreen());
	
	// If we haven't already, queue the processing of device state for next tick.
	if (!RequestProcessControllerStateHandle.IsValid())
	{
		RequestProcessControllerStateHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float DeltaTime)
		{
			RequestProcessControllerStateHandle.Reset();
			ProcessControllerDevicesHavingChangedForDisconnectScreen();
			return false;
		}));
	}
}

void UGameplayGameLayout::ProcessControllerDevicesHavingChangedForDisconnectScreen()
{
	// We should only ever get here if we have bound to the controller state change delegates
	ensure(ShouldPlatformDisplayControllerDisconnectScreen());
	
	const FPlatformUserId OwningLocalPlayerId = GetOwningLocalPlayer()->GetPlatformUserId();
	ensure(OwningLocalPlayerId.IsValid());
	
	// Get all input devices mapped to our player.
	const IPlatformInputDeviceMapper& DeviceMapper = IPlatformInputDeviceMapper::Get();
	TArray<FInputDeviceId> MappedInputDevices;
	const int32 NumDevicesMappedToUser = DeviceMapper.GetAllInputDevicesForUser(OwningLocalPlayerId, OUT MappedInputDevices);
	
	// Check if there are any other connected GAMEPAD devices mapped to this platform user.
	bool bHasConnectedController = false;
	
	for (const FInputDeviceId MappedDevice : MappedInputDevices)
	{
		if (DeviceMapper.GetInputDeviceConnectionState(MappedDevice) == EInputDeviceConnectionState::Connected)
		{
			const FHardwareDeviceIdentifier HardwareInfo = UInputDeviceSubsystem::Get()->GetInputDeviceHardwareIdentifier(MappedDevice);
			if (HardwareInfo.PrimaryDeviceType == EHardwareDevicePrimaryType::Gamepad)
			{
				bHasConnectedController = true;
			}
		}
	}
	
	// If there are no gamepad input devices mapped to this user, then we want to pop the toast saying to re-connect them
	if (!bHasConnectedController)
	{
		DisplayControllerDisconnectedMenu();
	}
	// Otherwise we can hide the screen if it is currently being shown
	else if (SpawnedControllerDisconnectedScreen)
	{
		HideControllerDisconnectedMenu();
	}
}

void UGameplayGameLayout::DisplayControllerDisconnectedMenu_Implementation()
{
	UE_LOG(LogGameplayGameLayout, Log, TEXT("[%hs] Display controller disconnected menu!"), __func__);
	
	if (ControllerDisconnectedScreen)
	{
		// Push the "controller disconnected" widget to the menu layer.
		SpawnedControllerDisconnectedScreen = UGameplayCommonUILibrary::PushActivatableWidgetForClass(GetOwningPlayer(), GameplayCommonTags::Layer_Menu, ControllerDisconnectedScreen);
	}
}

void UGameplayGameLayout::HideControllerDisconnectedMenu_Implementation()
{
	UE_LOG(LogGameplayGameLayout, Log, TEXT("[%hs] Hide controller disconnected menu!"), __func__);
	
	UGameplayCommonUILibrary::PopSingleWidget(SpawnedControllerDisconnectedScreen);
	SpawnedControllerDisconnectedScreen = nullptr;
}
