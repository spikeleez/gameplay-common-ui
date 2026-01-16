#pragma once

#include "AudioMixerBlueprintLibrary.h"
#include "Engine/LocalPlayer.h"
#include "Interfaces/GameplayCommonSettingsInterface.h"
#include "Engine/World.h"
#include "AudioMixerBlueprintLibrary.h"
#include "GameplaySettingsLocalPlayer.generated.h"

class UInputMappingContext;
class UGameplaySettingsShared;
class UGameplaySettingsLocal;

UCLASS()
class UGameplaySettingsLocalPlayer : public ULocalPlayer, public IGameplayCommonSettingsInterface
{
	GENERATED_BODY()
	
public:
	//~Begin ULocalPlayer interface
	virtual void PostInitProperties() override;
	//~End of ULocalPlayer interface
	
	/** Gets the local settings for this player, this is read from config files at process startup and is always valid */
	UFUNCTION()
	virtual UGameplaySettingsShared* GetSharedSettings() const override;
	
	/** Gets the shared setting for this player, this is read using the save game system so may not be correct until after user login */
	UFUNCTION()
	virtual UGameplaySettingsLocal* GetLocalSettings() const override;
	
	virtual void LoadSharedSettingsFromDisk(bool bForceLoad = false); 
	
protected:
	virtual void OnSharedSettingsLoaded(UGameplaySettingsShared* LoadedOrCreatedSettings);
	
	virtual void OnAudioOutputDeviceChanged(const FString& InAudioOutputDeviceId);
	
	UFUNCTION()
	virtual void OnCompletedAudioDeviceSwap(const FSwapAudioOutputResult& SwapResult);
	
protected:
	UPROPERTY(Transient)
	mutable TObjectPtr<UGameplaySettingsShared> SharedSettings;
	
	FUniqueNetIdRepl NetIdForSharedSettings;
	
	UPROPERTY(Transient)
	mutable TObjectPtr<const UInputMappingContext> InputMappingContext;
};
