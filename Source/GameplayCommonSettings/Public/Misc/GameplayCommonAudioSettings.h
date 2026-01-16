#pragma once

#include "Engine/DeveloperSettings.h"
#include "GameplayCommonAudioSettings.generated.h"

class USoundSubmix;
class USoundEffectSubmixPreset;

USTRUCT()
struct FGameplaySubmixEffectChainMap
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category="Audio", meta = (AllowedClasses = "/Script/Engine.SoundSubmix"))
	TSoftObjectPtr<USoundSubmix> Submix = nullptr;

	UPROPERTY(EditAnywhere, Category="Audio", meta = (AllowedClasses = "/Script/Engine.SoundEffectSubmixPreset"))
	TArray<TSoftObjectPtr<USoundEffectSubmixPreset>> SubmixEffectChain;
};

UCLASS(MinimalAPI, Config = Game, DefaultConfig, meta=(DisplayName = "Gameplay Common: Audio Settings"))
class UGameplayCommonAudioSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	/** The Default Base Control Bus Mix */
	UPROPERTY(Config, EditAnywhere, Category = "MixSettings", meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBusMix"))
	FSoftObjectPath DefaultControlBusMix;

	/** The Loading Screen Control Bus Mix - Called during loading screens to cover background audio events */
	UPROPERTY(Config, EditAnywhere, Category = "MixSettings", meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBusMix"))
	FSoftObjectPath LoadingScreenControlBusMix;

	/** The Default Base Control Bus Mix */
	UPROPERTY(Config, EditAnywhere, Category = "UserMixSettings", meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBusMix"))
	FSoftObjectPath UserSettingsControlBusMix;

	/** Control Bus assigned to the Overall sound volume setting */
	UPROPERTY(Config, EditAnywhere, Category = "UserMixSettings", meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
	FSoftObjectPath OverallVolumeControlBus;

	/** Control Bus assigned to the Music sound volume setting */
	UPROPERTY(Config, EditAnywhere, Category = "UserMixSettings", meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
	FSoftObjectPath MusicVolumeControlBus;

	/** Control Bus assigned to the SoundFX sound volume setting */
	UPROPERTY(Config, EditAnywhere, Category = "UserMixSettings", meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
	FSoftObjectPath SoundFXVolumeControlBus;

	/** Control Bus assigned to the Dialogue sound volume setting */
	UPROPERTY(Config, EditAnywhere, Category = "UserMixSettings", meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
	FSoftObjectPath DialogueVolumeControlBus;

	/** Control Bus assigned to the VoiceChat sound volume setting */
	UPROPERTY(Config, EditAnywhere, Category = "UserMixSettings", meta = (AllowedClasses = "/Script/AudioModulation.SoundControlBus"))
	FSoftObjectPath VoiceChatVolumeControlBus;

	/** Submix Processing Chains to achieve high dynamic range audio output */
	UPROPERTY(Config, EditAnywhere, Category = "EffectSettings")
	TArray<FGameplaySubmixEffectChainMap> HDRAudioSubmixEffectChain;
	
	/** Submix Processing Chains to achieve low dynamic range audio output */
	UPROPERTY(Config, EditAnywhere, Category = "EffectSettings")
	TArray<FGameplaySubmixEffectChainMap> LDRAudioSubmixEffectChain;
};
