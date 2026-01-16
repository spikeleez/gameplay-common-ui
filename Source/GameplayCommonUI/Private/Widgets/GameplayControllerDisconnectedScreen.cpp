// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayControllerDisconnectedScreen.h"
#include "CommonButtonBase.h"
#include "CommonUISettings.h"
#include "CommonUIVisibilitySubsystem.h"
#include "Misc/GameplayCommonTags.h"
#include "ICommonUIModule.h"
#include "Components/HorizontalBox.h"

DEFINE_LOG_CATEGORY_STATIC(LogGameplayControllerDisconnectedScreen, Log, All);

UGameplayControllerDisconnectedScreen::UGameplayControllerDisconnectedScreen(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// By default, only strict pairing platforms will need this button.
	PlatformSupportsUserChangeTags.AddTag(GameplayCommonTags::Traits_HasStrictControllerPairing);
}

void UGameplayControllerDisconnectedScreen::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	if (!HBox_SwitchUser)
	{
		UE_LOG(LogGameplayControllerDisconnectedScreen, Error, TEXT("Unable to find HBox_SwitchUser on Widget %s"), *GetNameSafe(this));
		return;
	}
	
	if (!Button_ChangeUser)
	{
		UE_LOG(LogGameplayControllerDisconnectedScreen, Error, TEXT("Unable to find Button_ChangeUser on Widget %s"), *GetNameSafe(this));
		return;
	}
	
	HBox_SwitchUser->SetVisibility(ESlateVisibility::Collapsed);
	Button_ChangeUser->SetVisibility(ESlateVisibility::Hidden);
	
	if (ShouldDisplayChangeUserButton())
	{
		// This is the platform user for "unpaired" input devices. Not every platform supports this, so
		// only set this to visible if the unpaired user is valid.
		const FPlatformUserId UnpairedUserId = IPlatformInputDeviceMapper::Get().GetUserForUnpairedInputDevices();
		if (UnpairedUserId.IsValid())
		{
			HBox_SwitchUser->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			Button_ChangeUser->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
	
	Button_ChangeUser->OnClicked().AddUObject(this, &ThisClass::HandleChangeUserClicked);
}

bool UGameplayControllerDisconnectedScreen::ShouldDisplayChangeUserButton() const
{
	bool bRequiresChangeUserButton = ICommonUIModule::GetSettings().GetPlatformTraits().HasAll(PlatformSupportsUserChangeTags);
	
	// Check the tags that we may be emulating in the editor too.
#if WITH_EDITOR
	const FGameplayTagContainer& PlatformEmulationTags = UCommonUIVisibilitySubsystem::Get(GetOwningLocalPlayer())->GetVisibilityTags();
	bRequiresChangeUserButton |= PlatformEmulationTags.HasAll(PlatformSupportsUserChangeTags);
#endif
	
	return bRequiresChangeUserButton;
}

void UGameplayControllerDisconnectedScreen::HandleChangeUserClicked()
{
	ensure(ShouldDisplayChangeUserButton());
	
	UE_LOG(LogGameplayControllerDisconnectedScreen, Log, TEXT("[%hs] Change user requested!"), __func__);
	
	const FPlatformUserId OwningPlayerId = GetOwningLocalPlayer()->GetPlatformUserId();
	const FInputDeviceId DeviceId = IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser(OwningPlayerId);
	
	FGenericPlatformApplicationMisc::ShowPlatformUserSelector(DeviceId, EPlatformUserSelectorFlags::Default, 
	[this](const FPlatformUserSelectionCompleteParams& Params)
	{
		HandleChangeUserCompleted(Params);
	});
}

void UGameplayControllerDisconnectedScreen::HandleChangeUserCompleted(const FPlatformUserSelectionCompleteParams& Params)
{
	UE_LOG(LogGameplayControllerDisconnectedScreen, Log, TEXT("[%hs] User change complete!"), __func__);
	
	// @TODO: Handle any user changing logic in your game here!
}
