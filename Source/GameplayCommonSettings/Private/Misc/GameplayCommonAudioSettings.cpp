// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Misc/GameplayCommonAudioSettings.h"
#include "SoundControlBusMix.h"
#include "SoundControlBus.h"
#include "Sound/SoundEffectSubmix.h"
#include "Sound/SoundSubmix.h"
#include "UObject/ConstructorHelpers.h"

UGameplayCommonAudioSettings::UGameplayCommonAudioSettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FObjectFinder<USoundControlBusMix> DefaultControlBusMixObject(TEXT("/GameplayCommonUI/Art/Sounds/Modulation/ControlBusMixes/CBM_BaseMix"));
	if (DefaultControlBusMixObject.Succeeded())
	{
		DefaultControlBusMix = DefaultControlBusMixObject.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundControlBusMix> LoadingScreenControlBusMixObject(TEXT("/GameplayCommonUI/Art/Sounds/Modulation/ControlBusMixes/CBM_LoadingScreenMix"));
	if (LoadingScreenControlBusMixObject.Succeeded())
	{
		LoadingScreenControlBusMix = LoadingScreenControlBusMixObject.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundControlBusMix> UserControlBusMixObject(TEXT("/GameplayCommonUI/Art/Sounds/Modulation/ControlBusMixes/CBM_UserMix"));
	if (UserControlBusMixObject.Succeeded())
	{
		UserSettingsControlBusMix = UserControlBusMixObject.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundControlBus> OverallVolumeControlBusObject(TEXT("/GameplayCommonUI/Art/Sounds/Modulation/ControlBuses/CB_Main"));
	if (OverallVolumeControlBusObject.Succeeded())
	{
		OverallVolumeControlBus = OverallVolumeControlBusObject.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundControlBus> MusicVolumeControlBusObject(TEXT("/GameplayCommonUI/Art/Sounds/Modulation/ControlBuses/CB_Music"));
	if (MusicVolumeControlBusObject.Succeeded())
	{
		MusicVolumeControlBus = MusicVolumeControlBusObject.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundControlBus> SoundEffectsVolumeControlBusObject(TEXT("/GameplayCommonUI/Art/Sounds/Modulation/ControlBuses/CB_SoundEffects"));
	if (SoundEffectsVolumeControlBusObject.Succeeded())
	{
		SoundEffectsVolumeControlBus = SoundEffectsVolumeControlBusObject.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundControlBus> DialogueVolumeControlBusObject(TEXT("/GameplayCommonUI/Art/Sounds/Modulation/ControlBuses/CB_Dialogue"));
	if (DialogueVolumeControlBusObject.Succeeded())
	{
		DialogueVolumeControlBus = DialogueVolumeControlBusObject.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundControlBus> VoiceChatVolumeControlBusObject(TEXT("/GameplayCommonUI/Art/Sounds/Modulation/ControlBuses/CB_VoiceChat"));
	if (VoiceChatVolumeControlBusObject.Succeeded())
	{
		VoiceChatVolumeControlBus = VoiceChatVolumeControlBusObject.Object;
	}
	
	static ConstructorHelpers::FObjectFinder<USoundSubmix> MainSubmixObject(TEXT("/GameplayCommonUI/Art/Sounds/Submixes/MainSubmix"));
	static ConstructorHelpers::FObjectFinder<USoundEffectSubmixPreset> MainDynamicSubmixEffectObject(TEXT("/GameplayCommonUI/Art/Sounds/Effects/DYN_MainDynamics"));
	if (MainSubmixObject.Succeeded() && MainDynamicSubmixEffectObject.Succeeded())
	{
		FGameplaySubmixEffectChainMap ChainMap;
		ChainMap.Submix = MainSubmixObject.Object;
		ChainMap.SubmixEffectChain.Add(MainDynamicSubmixEffectObject.Object);
		HDRAudioSubmixEffectChain.Add(ChainMap);
	}
	
	static ConstructorHelpers::FObjectFinder<USoundEffectSubmixPreset> LowMultiBandDynamicsSubmixEffectObject(TEXT("/GameplayCommonUI/Art/Sounds/Effects/DYN_LowMultiBandDynamics"));
	static ConstructorHelpers::FObjectFinder<USoundEffectSubmixPreset> LowDynamicsSubmixEffectObject(TEXT("/GameplayCommonUI/Art/Sounds/Effects/DYN_LowDynamics"));
	if (MainSubmixObject.Succeeded() && LowMultiBandDynamicsSubmixEffectObject.Succeeded() && LowDynamicsSubmixEffectObject.Succeeded())
	{
		FGameplaySubmixEffectChainMap ChainMap;
		ChainMap.Submix = MainSubmixObject.Object;
		ChainMap.SubmixEffectChain.Add(LowMultiBandDynamicsSubmixEffectObject.Object);
		ChainMap.SubmixEffectChain.Add(LowDynamicsSubmixEffectObject.Object);
		LDRAudioSubmixEffectChain.Add(ChainMap);
	}
}
