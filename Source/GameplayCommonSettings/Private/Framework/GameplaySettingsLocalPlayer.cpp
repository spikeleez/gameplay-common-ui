#include "Framework/GameplaySettingsLocalPlayer.h"
#include "Framework/GameplaySettingsLocal.h"
#include "Framework/GameplaySettingsShared.h"

void UGameplaySettingsLocalPlayer::PostInitProperties()
{
	Super::PostInitProperties();
	
	if (UGameplaySettingsLocal* LocalSettings = GetLocalSettings())
	{
		LocalSettings->OnAudioOutputDeviceChanged.AddUObject(this, &ThisClass::OnAudioOutputDeviceChanged);
	}
}

UGameplaySettingsShared* UGameplaySettingsLocalPlayer::GetSharedSettings() const
{
	if (!SharedSettings)
	{
		// On PC, it's okay to use the sync load because it only checks the disk
		// This could use a platform tag to check for proper save support instead
		bool bCanLoadBeforeLogin = PLATFORM_DESKTOP;
		
		if (bCanLoadBeforeLogin)
		{
			SharedSettings = UGameplaySettingsShared::LoadOrCreateSettings(this);
		}
		else
		{
			// We need to wait for user login to get the real settings so return temp ones
			SharedSettings = UGameplaySettingsShared::CreateTemporarySettings(this);
		}
	}

	return SharedSettings;
}

UGameplaySettingsLocal* UGameplaySettingsLocalPlayer::GetLocalSettings() const
{
	return UGameplaySettingsLocal::Get();
}

void UGameplaySettingsLocalPlayer::LoadSharedSettingsFromDisk(bool bForceLoad)
{
	const FUniqueNetIdRepl CurrentNetId = GetCachedUniqueNetId();
	if (!bForceLoad && SharedSettings && CurrentNetId == NetIdForSharedSettings)
	{
		// Already loaded once, don't reload
		return;
	}
	ensure(UGameplaySettingsShared::AsyncLoadOrCreateSettings(this, UGameplaySettingsShared::FOnSettingsLoadedEvent::CreateUObject(this, &ThisClass::OnSharedSettingsLoaded)));
}

void UGameplaySettingsLocalPlayer::OnSharedSettingsLoaded(UGameplaySettingsShared* LoadedOrCreatedSettings)
{
	// The settings are applied before it gets here
	if (ensure(LoadedOrCreatedSettings))
	{
		// This will replace the temporary or previously loaded object which will GC out normally
		SharedSettings = LoadedOrCreatedSettings;

		NetIdForSharedSettings = GetCachedUniqueNetId();
	}
}

void UGameplaySettingsLocalPlayer::OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId)
{
	FOnCompletedDeviceSwap DevicesSwappedCallback;
	DevicesSwappedCallback.BindUFunction(this, FName("OnCompletedAudioDeviceSwap"));
	UAudioMixerBlueprintLibrary::SwapAudioOutputDevice(GetWorld(), InAudioOutputDeviceId, DevicesSwappedCallback);
}

void UGameplaySettingsLocalPlayer::OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult)
{
	if (SwapResult.Result == ESwapAudioOutputDeviceResultState::Failure)
	{
		// ...
	}
}
