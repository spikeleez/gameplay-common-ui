#include "Subsystems/GameplaySettingsLocalPlayerSubsystem.h"
#include "Framework/GameplaySettingsLocalPlayer.h"

void UGameplaySettingsLocalPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	if (UGameplaySettingsLocalPlayer* LocalPlayer = GetLocalPlayer<UGameplaySettingsLocalPlayer>())
	{
		LocalPlayer->LoadSharedSettingsFromDisk();
	}
}
