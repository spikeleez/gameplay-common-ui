#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Engine/World.h"                   
#include "Sound/SoundSubmix.h"
#include "Sound/SoundEffectSubmix.h"            
#include "AudioModulationStatics.h"               
#include "AudioMixerBlueprintLibrary.h"
#include "GameplayAudioMixEffectsSubsystem.generated.h"

class USoundSubmix;
class USoundEffectSubmixPreset;
class USoundControlBus;
class USoundControlBusMix;

USTRUCT()
struct FGameplayAudioSubmixEffectsChain
{
	GENERATED_BODY()

	// Submix on which to apply the Submix Effect Chain Override
	UPROPERTY(Transient)
	TObjectPtr<USoundSubmix> Submix = nullptr;

	// Submix Effect Chain Override (Effects processed in Array index order)
	UPROPERTY(Transient)
	TArray<TObjectPtr<USoundEffectSubmixPreset>> SubmixEffectChain;
};

/**
 * This subsystem is meant to automatically engage default and user control bus mixes
 * to retrieve previously saved user settings and apply them to the activated user mix.
 * Additionally, this subsystem will automatically apply HDR/LDR Audio Submix Effect Chain Overrides
 * based on the user's preference for HDR Audio. Submix Effect Chain Overrides are defined in the
 * Gameplay Audio Settings.
 */
UCLASS(MinimalAPI)
class UGameplayAudioMixEffectsSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	//~Begin USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;
	//~End of USubsystem interface
	
	/** Called once all UWorldSubsystems have been initialized */
	virtual void PostInitialize() override;
	
	/** Called when world is ready to start gameplay before the game mode transitions to the correct state and call BeginPlay on all actors */
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;

	/** Set whether the HDR Audio Submix Effect Chain Override settings are applied */
	void ApplyDynamicRangeEffectsChains(bool bHDRAudio);
	
protected:
	//void OnLoadingScreenStatusChanged(bool bShowingLoadingScreen);
	//void ApplyOrRemoveLoadingScreenMix(bool bWantsLoadingScreenMix);
	
	// Called when determining whether to create this Subsystem
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	// Default Sound Control Bus Mix retrieved from the Gameplay Audio Settings
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBusMix> DefaultBaseMix = nullptr;

	// Loading Screen Sound Control Bus Mix retrieved from the Gameplay Audio Settings
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBusMix> LoadingScreenMix = nullptr;

	// User Sound Control Bus Mix retrieved from the Gameplay Audio Settings
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBusMix> UserMix = nullptr;

	// Overall Sound Control Bus retrieved from the Gameplay Audio Settings and linked to the UI and game settings in GameplaySettingsLocal
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBus> OverallControlBus = nullptr;

	// Music Sound Control Bus retrieved from the Gameplay Audio Settings and linked to the UI and game settings in GameplaySettingsLocal
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBus> MusicControlBus = nullptr;

	// SoundFX Sound Control Bus retrieved from the Gameplay Audio Settings and linked to the UI and game settings in GameplaySettingsLocal
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBus> SoundFXControlBus = nullptr;

	// Dialogue Sound Control Bus retrieved from the Gameplay Audio Settings and linked to the UI and game settings in GameplaySettingsLocal
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBus> DialogueControlBus = nullptr;

	// VoiceChat Sound Control Bus retrieved from the Gameplay Audio Settings and linked to the UI and game settings in GameplaySettingsLocal
	UPROPERTY(Transient)
	TObjectPtr<USoundControlBus> VoiceChatControlBus = nullptr;

	// Submix Effect Chain Overrides to apply when HDR Audio is turned on
	UPROPERTY(Transient)
	TArray<FGameplayAudioSubmixEffectsChain> HDRSubmixEffectChain;

	// Submix Effect hain Overrides to apply when HDR Audio is turned off
	UPROPERTY(Transient)
	TArray<FGameplayAudioSubmixEffectsChain> LDRSubmixEffectChain;

	bool bAppliedLoadingScreenMix = false;
};
