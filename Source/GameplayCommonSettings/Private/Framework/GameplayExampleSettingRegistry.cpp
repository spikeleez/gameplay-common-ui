#include "Framework/GameplayExampleSettingRegistry.h"

#include "CommonInputBaseTypes.h"
#include "EnhancedInputSubsystems.h"
#include "Framework/GameplaySettingAction.h"
#include "Framework/GameplaySettingCollection.h"
#include "Framework/GameplaySettingsLocal.h"
#include "Framework/GameplaySettingsLocalPlayer.h"
#include "Framework/GameplaySettingValueDiscreteDynamic.h"
#include "Framework/GameplaySettingValueScalarDynamic.h"
#include "Framework/GameplaySettingsShared.h"

#include "EditCondition/GameplaySettingWhenCondition.h"
#include "EditCondition/GameplaySettingWhenPlatformHasTrait.h"
#include "EditCondition/GameplaySettingWhenPlayingAsPrimaryPlayer.h"

#include "Misc/GameplayCommonSettingsTags.h"
#include "Misc/GameplayCommonSettingsLibrary.h"
#include "Misc/GameplayCommonSettingsTypes.h"

#include "DataSource/GameplaySettingDataSourceDynamic.h"
#include "Framework/GameplaySettingValueKeyboard.h"
#include "UserSettings/EnhancedInputUserSettings.h"

#define LOCTEXT_NAMESPACE "GameplayExampleSettingRegistry"

UGameplayExampleSettingRegistry::UGameplayExampleSettingRegistry()
{
}

void UGameplayExampleSettingRegistry::OnInitialize(ULocalPlayer* InLocalPlayer)
{
	VideoSettings = InitializeVideoSettings(InLocalPlayer);
	InitializeVideoSettings_FrameRates(VideoSettings, InLocalPlayer);
	RegisterSetting(VideoSettings);
	
	AudioSettings = InitializeAudioSettings(InLocalPlayer);
	RegisterSetting(AudioSettings);
	
	GameplaySettings = InitializeGameplaySettings(InLocalPlayer);
	RegisterSetting(GameplaySettings);
	
	MouseAndKeyboardSettings = InitializeMouseAndKeyboardSettings(InLocalPlayer);
	RegisterSetting(MouseAndKeyboardSettings);
	
	GamepadSettings = InitializeGamepadSettings(InLocalPlayer);
	RegisterSetting(GamepadSettings);
}

//============================================================================
// VIDEO SETTINGS
//============================================================================

UGameplaySettingCollection* UGameplayExampleSettingRegistry::InitializeVideoSettings(ULocalPlayer* InLocalPlayer)
{
	UGameplaySettingCollection* Screen = NewObject<UGameplaySettingCollection>();
	Screen->SetDevName(TEXT("VideoCollection"));
	Screen->SetDisplayName(LOCTEXT("VideoCollection_Name", "Video"));
	Screen->Initialize(InLocalPlayer);

	UGameplaySettingValueDiscreteDynamic_Enum* WindowModeSetting;

	//----------------------------------------------------------------------------------
	// DISPLAY
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* Display = NewObject<UGameplaySettingCollection>();
		Display->SetDevName(TEXT("DisplayCollection"));
		Display->SetDisplayName(LOCTEXT("DisplayCollection_Name", "Display"));
		Screen->AddSetting(Display);

		//----------------------------------------------------------------------------------
		// Window Mode
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Enum>();
			Setting->SetDevName(TEXT("WindowMode"));
			Setting->SetDisplayName(LOCTEXT("WindowMode_Name", "Window Mode"));
			Setting->SetDescriptionRichText(LOCTEXT("WindowMode_Description", "In Windowed mode you can interact with other windows more easily, and drag the edges of the window to set the size. In Windowed Fullscreen mode you can easily switch between applications. In Fullscreen mode you cannot interact with other windows as easily, but the game will run slightly faster."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFullscreenMode));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFullscreenMode));
			Setting->AddOption(EWindowMode::Fullscreen, LOCTEXT("WindowModeFullscreen", "Fullscreen"));
			Setting->AddOption(EWindowMode::WindowedFullscreen, LOCTEXT("WindowModeWindowedFullscreen", "Windowed Fullscreen"));
			Setting->AddOption(EWindowMode::Windowed, LOCTEXT("WindowModeWindowed", "Windowed"));

			Setting->AddEditCondition(FGameplaySettingWhenPlatformHasTrait::KillIfMissing(GameplayCommonSettingsTags::Trait_SupportsWindowedMode, TEXT("Platform does not support window mode")));

			WindowModeSetting = Setting;

			Display->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Resolution
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscrete_Resolution* Setting = NewObject<UGameplaySettingValueDiscrete_Resolution>();
			Setting->SetDevName(TEXT("Resolution"));
			Setting->SetDisplayName(LOCTEXT("Resolution_Name", "Resolution"));
			Setting->SetDescriptionRichText(LOCTEXT("Resolution_Description", "Display Resolution determines the size of the window in Windowed mode. In Fullscreen mode, Display Resolution determines the graphics card output resolution, which can result in black bars depending on monitor and graphics card. Display Resolution is inactive in Windowed Fullscreen mode."));

			Setting->AddEditDependency(WindowModeSetting);
			Setting->AddEditCondition(FGameplaySettingWhenPlatformHasTrait::KillIfMissing(GameplayCommonSettingsTags::Trait_SupportsWindowedMode, TEXT("Platform does not support window mode")));
			Setting->AddEditCondition(MakeShared<FGameplaySettingWhenCondition>([WindowModeSetting](const ULocalPlayer*, FGameplaySettingEditableState& InOutEditState)
			{
				if (WindowModeSetting->GetValue<EWindowMode::Type>() == EWindowMode::WindowedFullscreen)
				{
					InOutEditState.Disable(LOCTEXT("ResolutionWindowedFullscreen_Disabled", "When the Window Mode is set to <strong>Windowed Fullscreen</>, the resolution must match the native desktop resolution."));
				}
			}));

			Display->AddSetting(Setting);
		}
		//-----------------------------------------------------------------------------------
		// Color Blind Mode
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Enum>();
			Setting->SetDevName(TEXT("ColorBlindMode"));
			Setting->SetDisplayName(LOCTEXT("ColorBlindMode_Name", "Color Blind Mode"));
			Setting->SetDescriptionRichText(LOCTEXT("ColorBlindMode_Description", "Using the provided images, test out the different color blind modes to find a color correction that works best for you."));
			
			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetColorBlindMode));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetColorBlindMode));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetColorBlindMode());
			Setting->AddOption(EGameplayColorBlindMode::Off, LOCTEXT("ColorBlindRotatorSettingOff", "Off"));
			Setting->AddOption(EGameplayColorBlindMode::Deuteranope, LOCTEXT("ColorBlindRotatorSettingDeuteranope", "Deuteranope"));
			Setting->AddOption(EGameplayColorBlindMode::Protanope, LOCTEXT("ColorBlindRotatorSettingProtanope", "Protanope"));
			Setting->AddOption(EGameplayColorBlindMode::Tritanope, LOCTEXT("ColorBlindRotatorSettingTritanope", "Tritanope"));

			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());

			Display->AddSetting(Setting);
		}
		//-----------------------------------------------------------------------------------
		// Color Blind Strength
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("ColorBlindStrength"));
			Setting->SetDisplayName(LOCTEXT("ColorBlindStrength_Name", "Color Blind Strength"));
			Setting->SetDescriptionRichText(LOCTEXT("ColorBlindStrength_Description", "Using the provided images, test out the different strengths to find a color correction that works best for you."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetColorBlindStrength));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetColorBlindStrength));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetColorBlindStrength());
			for (int32 Index = 0; Index <= 10; Index++)
			{
				Setting->AddOption(Index, FText::AsNumber(Index));
			}

			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());

			Display->AddSetting(Setting);
		}
		//-----------------------------------------------------------------------------------
		// Display Gamma
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueScalarDynamic* Setting = NewObject<UGameplaySettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("Brightness"));
			Setting->SetDisplayName(LOCTEXT("Brightness_Name", "Brightness"));
			Setting->SetDescriptionRichText(LOCTEXT("Brightness_Description", "Adjusts the brightness."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetDisplayGamma));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetDisplayGamma));
			Setting->SetDefaultValue(2.2);
			Setting->SetDisplayFormat([](double SourceValue, double NormalizedValue)
			{
				return FText::Format(LOCTEXT("BrightnessFormat", "{0}%"), (int32)FMath::GetMappedRangeValueClamped(FVector2D(0, 1), FVector2D(50, 150), NormalizedValue));
			});
			Setting->SetSourceRangeAndStep(TRange<double>(1.7, 2.7), 0.01);

			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());
			Setting->AddEditCondition(FGameplaySettingWhenPlatformHasTrait::KillIfMissing(GameplayCommonSettingsTags::Trait_NeedsBrightnessAdjustment, TEXT("Platform does not require brightness adjustment.")));

			Display->AddSetting(Setting);
		}
	}

	//----------------------------------------------------------------------------------
	// GRAPHICS QUALITY
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* GraphicsQuality = NewObject<UGameplaySettingCollection>();
		GraphicsQuality->SetDevName(TEXT("GraphicsQuality"));
		GraphicsQuality->SetDisplayName(LOCTEXT("GraphicsQuality_Name", "Graphics Quality"));
		Screen->AddSetting(GraphicsQuality);

		UGameplaySetting* AutoSetQuality;
		UGameplaySetting* GraphicsQualityPresets;

		//----------------------------------------------------------------------------------
		// Auto-Set Quality
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingAction* Setting = NewObject<UGameplaySettingAction>();
			Setting->SetDevName(TEXT("AutoSetQuality"));
			Setting->SetDisplayName(LOCTEXT("AutoSetQuality_Name", "Auto-Set Quality"));
			Setting->SetDescriptionRichText(LOCTEXT("AutoSetQuality_Description", "Automatically configure the graphics quality options based on a benchmark of the hardware."));

			Setting->SetDoesActionDirtySettings(true);
			Setting->SetActionText(LOCTEXT("AutoSetQuality_Action", "Auto-Set"));
			Setting->SetCustomAction([](const ULocalPlayer* LocalPlayer)
			{
				const UGameplayPlatformSpecificRenderingSettings* PlatformSettings = UGameplayPlatformSpecificRenderingSettings::Get();
				if (PlatformSettings->FramePacingMode != EGameplayFramePacingMode::MobileStyle)
				{
					// We don't save state until users apply the settings.
					constexpr bool bImmediatelySaveState = false;
					UGameplayCommonSettingsLibrary::GetLocalSettings(LocalPlayer)->RunAutoBenchmark(bImmediatelySaveState);
				}
			});

			Setting->AddEditCondition(MakeShared<FGameplaySettingWhenCondition>([](const ULocalPlayer* LocalPlayer, FGameplaySettingEditableState& InOutEditState)
			{
				//const UGameplayPlatformSpecificRenderingSettings* PlatformSettings = UGameplayPlatformSpecificRenderingSettings::Get();
				const bool bCanBenchmark = UGameplayCommonSettingsLibrary::GetLocalSettings(LocalPlayer)->CanRunAutoBenchmark();
				if (!bCanBenchmark)
				{
					InOutEditState.Kill(TEXT("Auto quality not supported"));
				}
			}));

			GraphicsQuality->AddSetting(Setting);

			AutoSetQuality = Setting;
		}
		//----------------------------------------------------------------------------------
		// Overall Quality
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscrete_OverallQuality* Setting = NewObject<UGameplaySettingValueDiscrete_OverallQuality>();
			Setting->SetDevName(TEXT("GraphicsQualityPresets"));
			Setting->SetDisplayName(LOCTEXT("GraphicsQualityPresets_Name", "Quality Presets"));
			Setting->SetDescriptionRichText(LOCTEXT("GraphicsQualityPresets_Description", "Quality Preset allows you to adjust multiple video options at once. Try a few options to see what fits your preference and device's performance."));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_FramePacingMode>(EGameplayFramePacingMode::ConsoleStyle, EGameplayFramePacingEditCondition::DisableIf));

			GraphicsQuality->AddSetting(Setting);

			GraphicsQualityPresets = Setting;
		}
		//----------------------------------------------------------------------------------
		// 3D Resolution
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueScalarDynamic* Setting = NewObject<UGameplaySettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("ResolutionScale"));
			Setting->SetDisplayName(LOCTEXT("ResolutionScale_Name", "3D Resolution"));
			Setting->SetDescriptionRichText(LOCTEXT("ResolutionScale_Description", "3D resolution determines the resolution that objects are rendered in game, but does not affect the main menu.  Lower resolutions can significantly increase frame rate."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetResolutionScaleNormalized));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetResolutionScaleNormalized));
			Setting->SetDisplayFormat(UGameplaySettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_VideoQuality>(TEXT("Platform does not support 3D Resolution")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);
			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Global Illumination
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("GlobalIlluminationQuality"));
			Setting->SetDisplayName(LOCTEXT("GlobalIlluminationQuality_Name", "Global Illumination"));
			Setting->SetDescriptionRichText(LOCTEXT("GlobalIlluminationQuality_Description", "Global Illumination controls the quality of dynamically calculated indirect lighting bounces, sky shadowing and Ambient Occlusion. Settings of 'High' and above use more accurate ray tracing methods to solve lighting, but can reduce performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetGlobalIlluminationQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetGlobalIlluminationQuality));
			Setting->AddOption(0, LOCTEXT("GlobalIlluminationQuality_Low", "Low"));
			Setting->AddOption(1, LOCTEXT("GlobalIlluminationQuality_Medium", "Medium"));
			Setting->AddOption(2, LOCTEXT("GlobalIlluminationQuality_High", "High"));
			Setting->AddOption(3, LOCTEXT("GlobalIlluminationQuality_Epic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_VideoQuality>(TEXT("Platform does not support GlobalIlluminationQuality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Shadows
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("Shadows"));
			Setting->SetDisplayName(LOCTEXT("Shadows_Name", "Shadows"));
			Setting->SetDescriptionRichText(LOCTEXT("Shadows_Description", "Shadow quality determines the resolution and view distance of dynamic shadows. Shadows improve visual quality and give better depth perception, but can reduce performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetShadowQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetShadowQuality));
			Setting->AddOption(0, LOCTEXT("Shadow_Low", "Off"));
			Setting->AddOption(1, LOCTEXT("Shadow_Medium", "Medium"));
			Setting->AddOption(2, LOCTEXT("Shadow_High", "High"));
			Setting->AddOption(3, LOCTEXT("Shadow_Epic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_VideoQuality>(TEXT("Platform does not support Shadows")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Anti-Aliasing
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("AntiAliasing"));
			Setting->SetDisplayName(LOCTEXT("AntiAliasing_Name", "Anti-Aliasing"));
			Setting->SetDescriptionRichText(LOCTEXT("AntiAliasing_Description", "Anti-Aliasing reduces jaggy artifacts along geometry edges. Increasing this setting will make edges look smoother, but can reduce performance. Higher settings mean more anti-aliasing."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetAntiAliasingQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetAntiAliasingQuality));
			Setting->AddOption(0, LOCTEXT("AntiAliasing_Low", "Off"));
			Setting->AddOption(1, LOCTEXT("AntiAliasing_Medium", "Medium"));
			Setting->AddOption(2, LOCTEXT("AntiAliasing_High", "High"));
			Setting->AddOption(3, LOCTEXT("AntiAliasing_Epic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_VideoQuality>(TEXT("Platform does not support Anti-Aliasing")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// View Distance
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("ViewDistance"));
			Setting->SetDisplayName(LOCTEXT("ViewDistance_Name", "View Distance"));
			Setting->SetDescriptionRichText(LOCTEXT("ViewDistance_Description", "View distance determines how far away objects are culled for performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetViewDistanceQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetViewDistanceQuality));
			Setting->AddOption(0, LOCTEXT("ViewDistance_Near", "Near"));
			Setting->AddOption(1, LOCTEXT("ViewDistance_Medium", "Medium"));
			Setting->AddOption(2, LOCTEXT("ViewDistance_Far", "Far"));
			Setting->AddOption(3, LOCTEXT("ViewDistance_Epic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_VideoQuality>(TEXT("Platform does not support View Distance")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Texture Quality
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("TextureQuality"));
			Setting->SetDisplayName(LOCTEXT("TextureQuality_Name", "Textures"));

			Setting->SetDescriptionRichText(LOCTEXT("TextureQuality_Description", "Texture quality determines the resolution of textures in game. Increasing this setting will make objects more detailed, but can reduce performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetTextureQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetTextureQuality));
			Setting->AddOption(0, LOCTEXT("TextureQuality_Low", "Low"));
			Setting->AddOption(1, LOCTEXT("TextureQuality_Medium", "Medium"));
			Setting->AddOption(2, LOCTEXT("TextureQuality_High", "High"));
			Setting->AddOption(3, LOCTEXT("TextureQuality_Epic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_VideoQuality>(TEXT("Platform does not support Texture quality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Visual Effects Quality
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("VisualEffectQuality"));
			Setting->SetDisplayName(LOCTEXT("VisualEffectQuality_Name", "Effects"));
			Setting->SetDescriptionRichText(LOCTEXT("VisualEffectQuality_Description", "Effects determines the quality of visual effects and lighting in game. Increasing this setting will increase the quality of visual effects, but can reduce performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetVisualEffectQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetVisualEffectQuality));
			Setting->AddOption(0, LOCTEXT("VisualEffectQuality_Low", "Low"));
			Setting->AddOption(1, LOCTEXT("VisualEffectQuality_Medium", "Medium"));
			Setting->AddOption(2, LOCTEXT("VisualEffectQuality_High", "High"));
			Setting->AddOption(3, LOCTEXT("VisualEffectQuality_Epic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_VideoQuality>(TEXT("Platform does not support VisualEffectQuality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Reflection Quality
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("ReflectionQuality"));
			Setting->SetDisplayName(LOCTEXT("ReflectionQuality_Name", "Reflections"));
			Setting->SetDescriptionRichText(LOCTEXT("ReflectionQuality_Description", "Reflection quality determines the resolution and accuracy of reflections.  Settings of 'High' and above use more accurate ray tracing methods to solve reflections, but can reduce performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetReflectionQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetReflectionQuality));
			Setting->AddOption(0, LOCTEXT("VisualEffectQuality_Low", "Low"));
			Setting->AddOption(1, LOCTEXT("VisualEffectQuality_Medium", "Medium"));
			Setting->AddOption(2, LOCTEXT("VisualEffectQuality_High", "High"));
			Setting->AddOption(3, LOCTEXT("VisualEffectQuality_Epic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_VideoQuality>(TEXT("Platform does not support ReflectionQuality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Post Processing Quality
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
			Setting->SetDevName(TEXT("PostProcessingQuality"));
			Setting->SetDisplayName(LOCTEXT("PostProcessingQuality_Name", "Post Processing"));
			Setting->SetDescriptionRichText(LOCTEXT("PostProcessingQuality_Description", "Post Processing effects include Motion Blur, Depth of Field and Bloom. Increasing this setting improves the quality of post process effects, but can reduce performance."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetPostProcessingQuality));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetPostProcessingQuality));
			Setting->AddOption(0, LOCTEXT("PostProcessingQuality_Low", "Low"));
			Setting->AddOption(1, LOCTEXT("PostProcessingQuality_Medium", "Medium"));
			Setting->AddOption(2, LOCTEXT("PostProcessingQuality_High", "High"));
			Setting->AddOption(3, LOCTEXT("PostProcessingQuality_Epic", "Epic"));

			Setting->AddEditDependency(AutoSetQuality);
			Setting->AddEditDependency(GraphicsQualityPresets);
			Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_VideoQuality>(TEXT("Platform does not support PostProcessingQuality")));

			// When this setting changes, it can GraphicsQualityPresets to be set to custom, or a particular preset.
			GraphicsQualityPresets->AddEditDependency(Setting);

			GraphicsQuality->AddSetting(Setting);
		}
	}
	//----------------------------------------------------------------------------------
	// ADVANCED GRAPHICS
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* AdvancedGraphics = NewObject<UGameplaySettingCollection>();
		AdvancedGraphics->SetDevName(TEXT("AdvancedGraphics"));
		AdvancedGraphics->SetDisplayName(LOCTEXT("AdvancedGraphics_Name", "Advanced Graphics"));
		Screen->AddSetting(AdvancedGraphics);

		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("VerticalSync"));
			Setting->SetDisplayName(LOCTEXT("VerticalSync_Name", "Vertical Sync"));
			Setting->SetDescriptionRichText(LOCTEXT("VerticalSync_Description", "Enabling Vertical Sync eliminates screen tearing by always rendering and presenting a full frame. Disabling Vertical Sync can give higher frame rate and better input response, but can result in horizontal screen tearing."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(IsVSyncEnabled));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetVSyncEnabled));
			Setting->SetDefaultValue(false);

			Setting->AddEditDependency(WindowModeSetting);

			Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_FramePacingMode>(EGameplayFramePacingMode::DesktopStyle));
			Setting->AddEditCondition(MakeShared<FGameplaySettingWhenCondition>([WindowModeSetting](const ULocalPlayer*, FGameplaySettingEditableState& InOutEditState)
			{
				if (WindowModeSetting->GetValue<EWindowMode::Type>() != EWindowMode::Fullscreen)
				{
					InOutEditState.Disable(LOCTEXT("FullscreenNeededForVSync", "This feature only works if 'Window Mode' is set to 'Fullscreen'."));
				}
			}));

			AdvancedGraphics->AddSetting(Setting);
		}
	}

	return Screen;
}

void UGameplayExampleSettingRegistry::AddPerformanceStatPage(UGameplaySettingCollection* PerfStatsOuterCategory, ULocalPlayer* InLocalPlayer)
{
	//----------------------------------------------------------------------------------
	// STATS PAGE
	//----------------------------------------------------------------------------------
	{
		static_assert((int32)EGameplayDisplayablePerformanceStat::MAX == 18, "Consider updating this function to deal with new performance stats.");

		UGameplaySettingCollectionPage* StatsPage = NewObject<UGameplaySettingCollectionPage>();
		StatsPage->SetDevName(TEXT("PerfStatsPage"));
		StatsPage->SetDisplayName(LOCTEXT("PerfStatsPage_Name", "Performance Stats"));
		StatsPage->SetDescriptionRichText(LOCTEXT("PerfStatsPage_Description", "Configure the display of performance statistics."));
		StatsPage->SetNavigationText(LOCTEXT("PerfStatsPage_Navigation", "Edit"));

		StatsPage->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());

		PerfStatsOuterCategory->AddSetting(StatsPage);

		//----------------------------------------------------------------------------------
		// PERFORMANCE STATS
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingCollection* StatsCategory_Performance = NewObject<UGameplaySettingCollection>();
			StatsCategory_Performance->SetDevName(TEXT("StatsCategory_Performance"));
			StatsCategory_Performance->SetDisplayName(LOCTEXT("StatCategory_Performance_Name", "Performance"));
			StatsPage->AddSetting(StatsCategory_Performance);

			//----------------------------------------------------------------------------------
			// Client FPS
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::ClientFPS);
				Setting->SetDisplayName(LOCTEXT("PerfStat_ClientFPS_Name", "Client FPS"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_ClientFPS_Description", "Client frame rate (higher is better)."));
				StatsCategory_Performance->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// Server FPS
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::ServerFPS);
				Setting->SetDisplayName(LOCTEXT("PerfStat_ServerFPS_Name", "Server FPS"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_ServerFPS_Description", "Server frame rate."));
				StatsCategory_Performance->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// FrameTime Total
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::FrameTime_Total);
				Setting->SetDisplayName(LOCTEXT("PerfStat_FrameTime_Name", "Frame Time (Total)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_FrameTime_Description", "The total frame time."));
				StatsCategory_Performance->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// Idle Time
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::IdleTime);
				Setting->SetDisplayName(LOCTEXT("PerfStat_IdleTime_Name", "Idle Time"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_IdleTime_Description", "The amount of time spent waiting idle for frame pacing."));
				StatsCategory_Performance->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// FrameTime (Game Thread)
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::FrameTime_GameThread);
				Setting->SetDisplayName(LOCTEXT("PerfStat_FrameTime_GameThread_Name", "Frame Time (Game Thread)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_FrameTime_GameThread_Description", "The amount of time spent on the main game thread."));
				StatsCategory_Performance->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// FrameTime (Render Thread)
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::FrameTime_RenderThread);
				Setting->SetDisplayName(LOCTEXT("PerfStat_FrameTime_RenderThread_Name", "Frame Time (Render Thread)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_FrameTime_RenderThread_Description", "The amount of time spent on the rendering thread."));
				StatsCategory_Performance->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// FrameTime (RHI Thread)
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::FrameTime_RHIThread);
				Setting->SetDisplayName(LOCTEXT("PerfStat_FrameTime_RHIThread_Name", "Frame Time (RHI Thread)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_FrameTime_RHIThread_Description", "The amount of time spent on the Render Hardware Interface (RHI) thread."));
				StatsCategory_Performance->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// FrameTime (GPU Thread)
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::FrameTime_GPU);
				Setting->SetDisplayName(LOCTEXT("PerfStat_FrameTime_GPU_Name", "Frame Time (GPU)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_FrameTime_GPU_Description", "The amount of time spent on the GPU"));
				StatsCategory_Performance->AddSetting(Setting);
			}
		}
		//----------------------------------------------------------------------------------
		// NETWORK STATS
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingCollection* StatCategory_Network = NewObject<UGameplaySettingCollection>();
			StatCategory_Network->SetDevName(TEXT("StatCategory_Network"));
			StatCategory_Network->SetDisplayName(LOCTEXT("StatCategory_Network_Name", "Network"));
			StatsPage->AddSetting(StatCategory_Network);

			//----------------------------------------------------------------------------------
			// Ping (ms)
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::Ping);
				Setting->SetDisplayName(LOCTEXT("PerfStat_Ping_Name", "Ping (ms)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_Ping_Description", "The roundtrip latency of tour connection to the server."));
				StatCategory_Network->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// PacketLoss (Incoming)
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::PacketLoss_Incoming);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketLoss_Incoming_Name", "Packet Loss (Incoming)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_PacketLoss_Incoming_Description", "The percentage of incoming packet lost."));
				StatCategory_Network->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// PacketLoss (Outgoing)
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::PacketLoss_Outgoing);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketLoss_Outgoing_Name", "Packet Loss (Outgoing)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_PacketLoss_Outgoing_Description", "The percentage of outgoing packet lost."));
				StatCategory_Network->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// PacketRate (Incoming)
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::PacketRate_Incoming);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketRate_Incoming_Name", "Packet Rate (Incoming)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_PacketRate_Outgoing_Description", "Rate of incoming packets (per second)."));
				StatCategory_Network->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// PacketRate (Outgoing)
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::PacketRate_Outgoing);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketRate_Outgoing_Name", "Packet Rate (Outgoing)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_PacketRate_Outgoing_Description", "Rate of outgoing packets (per second)."));
				StatCategory_Network->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// PacketSize (Incoming)
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::PacketSize_Incoming);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketSize_Incoming_Name", "Packet Size (Incoming)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_PacketSize_Incoming_Description", "The average size (in bytes) of packets received in the last second."));
				StatCategory_Network->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// PacketSize (Outgoing)
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::PacketSize_Outgoing);
				Setting->SetDisplayName(LOCTEXT("PerfStat_PacketSize_Outgoing_Name", "Packet Size (Outgoing)"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStat_PacketSize_Outgoing_Description", "The average size (in bytes) of packets sent in the last second."));
				StatCategory_Network->AddSetting(Setting);
			}
		}

		//----------------------------------------------------------------------------------
		// LATENCY STATS
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingCollection* StatCategory_Latency = NewObject<UGameplaySettingCollection>();
			StatCategory_Latency->SetDevName(TEXT("StatCategory_Latency"));
			StatCategory_Latency->SetDisplayName(LOCTEXT("StatCategory_Latency_Name", "Latency"));
			StatsPage->AddSetting(StatCategory_Latency);

			//----------------------------------------------------------------------------------
			// Latency Tracking
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Bool>();
				Setting->SetDevName(TEXT("InputLatencyTrackingStats"));
				Setting->SetDisplayName(LOCTEXT("InputLatencyTrackingStats_Name", "Enable Latency Tracking Stats"));
				Setting->SetDescriptionRichText(LOCTEXT("InputLatencyTrackingStats_Description", "Enabling Input Latency stat tracking"));

				Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetEnableLatencyTrackingStats));
				Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetEnableLatencyTrackingStats));

				// Set the default value to true if the platform supports latency tracking stats
				Setting->SetDefaultValue(UGameplaySettingsLocal::DoesPlatformSupportLatencyTrackingStats());
				Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_LatencyStatsSupported>());

				StatCategory_Latency->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// Latency Markers
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Bool>();
				Setting->SetDevName(TEXT("InputLatencyMarkers"));
				Setting->SetDisplayName(LOCTEXT("InputLatencyMarkers_Name", "Enable Latency Markers"));
				Setting->SetDescriptionRichText(LOCTEXT("InputLatencyMarkers_Description", "Enabling Input Latency Markers to flash the screen"));

				Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetEnableLatencyFlashIndicators));
				Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetEnableLatencyFlashIndicators));
				Setting->SetDefaultValue(false);

				// Latency markers require the stats to be supported and enabled
				Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_LatencyStatsSupported>());
				Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_LatencyStatsCurrentlyEnabled>());
				Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_LatencyMarkersSupported>());

				StatCategory_Latency->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// Total Game Latency
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::Latency_Total);
				Setting->SetDisplayName(LOCTEXT("PerfStat_Latency_Total", "Total Game Latency"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_Latency_Total", "The total amount of latency"));
				Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_LatencyStatsSupported>());
				Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_LatencyStatsCurrentlyEnabled>());
				StatCategory_Latency->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// Game Latency
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::Latency_Game);
				Setting->SetDisplayName(LOCTEXT("PerfStat_Latency_Game", "Game Latency"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_Latency_Game", "Game simulation start to driver submission end"));
				Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_LatencyStatsSupported>());
				Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_LatencyStatsCurrentlyEnabled>());
				StatCategory_Latency->AddSetting(Setting);
			}
			//----------------------------------------------------------------------------------
			// Render Latency
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingValueDiscrete_PerfStat* Setting = NewObject<UGameplaySettingValueDiscrete_PerfStat>();
				Setting->SetStat(EGameplayDisplayablePerformanceStat::Latency_Render);
				Setting->SetDisplayName(LOCTEXT("PerfStat_Latency_Render", "Render Latency"));
				Setting->SetDescriptionRichText(LOCTEXT("PerfStatDescription_Latency_Render", "OS render queue start to GPU render end"));
				Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_LatencyStatsSupported>());
				Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_LatencyStatsCurrentlyEnabled>());
				StatCategory_Latency->AddSetting(Setting);
			}
		}
	}
}

void AddFrameRateOptions(UGameplaySettingValueDiscreteDynamic_Number* Setting)
{
	const FText FPSFormat = LOCTEXT("FPSFormat", "{0} FPS");
	for (const int32 Rate : GetDefault<UGameplayCommonPerformanceSettings>()->DesktopFrameRateLimits)
	{
		Setting->AddOption((float)Rate, FText::Format(FPSFormat, Rate));
	}
	Setting->AddOption(0.0f, LOCTEXT("UnlimitedFPS", "Unlimited"));
}

void UGameplayExampleSettingRegistry::InitializeVideoSettings_FrameRates(UGameplaySettingCollection* Screen, ULocalPlayer* InLocalPlayer)
{
	//----------------------------------------------------------------------------------
	// Frame Rate Limit (On Battery)
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
		Setting->SetDevName(TEXT("FrameRateLimit_OnBattery"));
		Setting->SetDisplayName(LOCTEXT("FrameRateLimit_OnBattery_Name", "Frame Rate Limit (On Battery)"));
		Setting->SetDescriptionRichText(LOCTEXT("FrameRateLimit_OnBattery_Description", "Frame rate limit when running on battery. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."));

		Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_OnBattery));
		Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_OnBattery));
		Setting->SetDefaultValue(GetDefault<UGameplaySettingsLocal>()->GetFrameRateLimit_OnBattery());

		Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_FramePacingMode>(EGameplayFramePacingMode::DesktopStyle));
		//#todo: Hide if this device doesn't have a battery (no API for this right now)

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
	//----------------------------------------------------------------------------------
	// Frame Rate Limit (In Menu)
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
		Setting->SetDevName(TEXT("FrameRateLimit_InMenu"));
		Setting->SetDisplayName(LOCTEXT("FrameRateLimit_InMenu_Name", "Frame Rate Limit (Menu)"));
		Setting->SetDescriptionRichText(LOCTEXT("FrameRateLimit_InMenu_Description", "Frame rate limit when in the menu. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."));

		Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_InMenu));
		Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_InMenu));
		Setting->SetDefaultValue(GetDefault<UGameplaySettingsLocal>()->GetFrameRateLimit_InMenu());
		Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_FramePacingMode>(EGameplayFramePacingMode::DesktopStyle));

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
	//----------------------------------------------------------------------------------
	// Frame Rate Limit (When Background)
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
		Setting->SetDevName(TEXT("FrameRateLimit_WhenBackgrounded"));
		Setting->SetDisplayName(LOCTEXT("FrameRateLimit_WhenBackgrounded_Name", "Frame Rate Limit (Background)"));
		Setting->SetDescriptionRichText(LOCTEXT("FrameRateLimit_WhenBackgrounded_Description", "Frame rate limit when in the background. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."));

		Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_WhenBackgrounded));
		Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_WhenBackgrounded));
		Setting->SetDefaultValue(GetDefault<UGameplaySettingsLocal>()->GetFrameRateLimit_WhenBackgrounded());
		Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_FramePacingMode>(EGameplayFramePacingMode::DesktopStyle));

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
	//----------------------------------------------------------------------------------
	// Frame Rate Limit (Always)
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingValueDiscreteDynamic_Number* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Number>();
		Setting->SetDevName(TEXT("FrameRateLimit_Always"));
		Setting->SetDisplayName(LOCTEXT("FrameRateLimit_Always_Name", "Frame Rate Limit"));
		Setting->SetDescriptionRichText(LOCTEXT("FrameRateLimit_Always_Description", "Frame rate limit sets the highest frame rate that is allowed. Set this lower for a more consistent frame rate or higher for the best experience on faster machines. You may need to disable Vsync to reach high frame rates."));

		Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetFrameRateLimit_Always));
		Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetFrameRateLimit_Always));
		Setting->SetDefaultValue(GetDefault<UGameplaySettingsLocal>()->GetFrameRateLimit_Always());
		Setting->AddEditCondition(MakeShared<FGameplaySettingEditCondition_FramePacingMode>(EGameplayFramePacingMode::DesktopStyle));

		AddFrameRateOptions(Setting);

		Screen->AddSetting(Setting);
	}
	
	{
		UGameplaySettingCollection* PerformanceSubCollection = NewObject<UGameplaySettingCollection>();
		PerformanceSubCollection->SetDevName(TEXT("PerformanceSubCollection"));
		PerformanceSubCollection->SetDisplayName(LOCTEXT("PerformanceSubCollection_Name", "Performance"));
		Screen->AddSetting(PerformanceSubCollection);
		
		//----------------------------------------------------------------------------------
		// Performance Stats Page
		//----------------------------------------------------------------------------------
		{
			AddPerformanceStatPage(PerformanceSubCollection, InLocalPlayer);
		}
	}
}

//============================================================================
// AUDIO SETTINGS
//============================================================================

UGameplaySettingCollection* UGameplayExampleSettingRegistry::InitializeAudioSettings(ULocalPlayer* InLocalPlayer)
{
	UGameplaySettingCollection* Screen = NewObject<UGameplaySettingCollection>();
	Screen->SetDevName(TEXT("AudioCollection"));
	Screen->SetDisplayName(LOCTEXT("AudioCollection_Name", "Audio"));
	Screen->Initialize(InLocalPlayer);

	//----------------------------------------------------------------------------------
	// Volume
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* VolumeSubCollection = NewObject<UGameplaySettingCollection>();
		VolumeSubCollection->SetDevName(TEXT("VolumeCollection"));
		VolumeSubCollection->SetDisplayName(LOCTEXT("VolumeCollection_Name", "Volume"));
		Screen->AddSetting(VolumeSubCollection);

		//----------------------------------------------------------------------------------
		// Overall Volume
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueScalarDynamic* Setting = NewObject<UGameplaySettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("OverallVolume"));
			Setting->SetDisplayName(LOCTEXT("OverallVolume_Name", "Overall"));
			Setting->SetDescriptionRichText(LOCTEXT("OverallVolume_Description", "Adjusts the volume of everything."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetOverallVolume));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetOverallVolume));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsLocal>()->GetOverallVolume());
			Setting->SetDisplayFormat(UGameplaySettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());

			VolumeSubCollection->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Music Volume
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueScalarDynamic* Setting = NewObject<UGameplaySettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("MusicVolume"));
			Setting->SetDisplayName(LOCTEXT("MusicVolume_Name", "Music"));
			Setting->SetDescriptionRichText(LOCTEXT("MusicVolume_Description", "Adjusts the volume of music."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetMusicVolume));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetMusicVolume));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsLocal>()->GetMusicVolume());
			Setting->SetDisplayFormat(UGameplaySettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());

			VolumeSubCollection->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Sound Effects Volume
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueScalarDynamic* Setting = NewObject<UGameplaySettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("SoundEffectsVolume"));
			Setting->SetDisplayName(LOCTEXT("SoundEffectsVolume_Name", "Sound Effects"));
			Setting->SetDescriptionRichText(LOCTEXT("SoundEffectsVolume_Description", "Adjusts the volume of sound effects."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetSoundEffectsVolume));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetSoundEffectsVolume));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsLocal>()->GetSoundEffectsVolume());
			Setting->SetDisplayFormat(UGameplaySettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());

			VolumeSubCollection->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Dialogue Volume
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueScalarDynamic* Setting = NewObject<UGameplaySettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("DialogueVolume"));
			Setting->SetDisplayName(LOCTEXT("DialogueVolume_Name", "Dialogue"));
			Setting->SetDescriptionRichText(LOCTEXT("DialogueVolume_Description", "Adjusts the volume of dialogue for game characters and voice overs."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetDialogueVolume));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetDialogueVolume));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsLocal>()->GetDialogueVolume());
			Setting->SetDisplayFormat(UGameplaySettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());

			VolumeSubCollection->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Voice Chat Volume
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueScalarDynamic* Setting = NewObject<UGameplaySettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("VoiceChatVolume"));
			Setting->SetDisplayName(LOCTEXT("VoiceChatVolume_Name", "Voice Chat"));
			Setting->SetDescriptionRichText(LOCTEXT("VoiceChatVolume_Description", "Adjusts the volume of voice chat."));

			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetVoiceChatVolume));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetVoiceChatVolume));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsLocal>()->GetVoiceChatVolume());
			Setting->SetDisplayFormat(UGameplaySettingValueScalarDynamic::ZeroToOnePercent);

			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());

			VolumeSubCollection->AddSetting(Setting);
		}
	}
	
	//----------------------------------------------------------------------------------
	// Audio Devices SubCollection
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* AudioDevicesSubCollection = NewObject<UGameplaySettingCollection>();
		AudioDevicesSubCollection->SetDevName(TEXT("AudioDevicesSubCollection"));
		AudioDevicesSubCollection->SetDisplayName(LOCTEXT("AudioDevicesSubCollection_Name", "Audio Devices"));
		Screen->AddSetting(AudioDevicesSubCollection);
		
		//----------------------------------------------------------------------------------
		// Audio Output Devices
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_AudioOutputDevice* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_AudioOutputDevice>();
			Setting->SetDevName(TEXT("AudioOutputDevice"));
			Setting->SetDisplayName(LOCTEXT("AudioOutputDevice_Name", "Audio Output Device"));
			Setting->SetDescriptionRichText(LOCTEXT("AudioOutputDevice_Description", "Changes the audio output device for game audio (not voice chat)."));
			
			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetAudioOutputDeviceId));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetAudioOutputDeviceId));
			
			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());
			Setting->AddEditCondition(FGameplaySettingWhenPlatformHasTrait::KillIfMissing(
				GameplayCommonSettingsTags::Trait_SupportsChangingAudioOutputDevice, TEXT("Platform does not support changing audio output device."))
			);
			
			AudioDevicesSubCollection->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Background Audio
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Enum>();
			Setting->SetDevName(TEXT("BackgroundAudio"));
			Setting->SetDisplayName(LOCTEXT("BackgroundAudio_Name", "Background Audio"));
			Setting->SetDescriptionRichText(LOCTEXT("BackgroundAudio_Description", "Turns game audio on/off when the game is in the background. When on, the game audio will continue to play when the game is minimized, or another window is focused."));
			
			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetAllowAudioInBackground));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetAllowAudioInBackground));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetAllowAudioInBackground());
			
			Setting->AddOption(EGameplayAllowBackgroundAudio::Off, LOCTEXT("BackgroundAudio_Off", "Off"));
			Setting->AddOption(EGameplayAllowBackgroundAudio::AllSounds, LOCTEXT("BackgroundAudio_AllSounds", "All Sounds"));
			
			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());
			Setting->AddEditCondition(FGameplaySettingWhenPlatformHasTrait::KillIfMissing(GameplayCommonSettingsTags::Trait_SupportsBackgroundAudio, TEXT("Platform does not support background audio.")));
			
			AudioDevicesSubCollection->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Headphone Mode
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("HeadphoneMode"));
			Setting->SetDisplayName(LOCTEXT("HeadphoneMode_Name", "3D Headphones"));
			Setting->SetDescriptionRichText(LOCTEXT("HeadphoneMode_Description", "Enable binaural audio.  Provides 3D audio spatialization, so you can hear the location of sounds more precisely, including above, below, and behind you. Recommended for use with stereo headphones only."));
			
			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetDesiredHeadphoneMode));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetDesiredHeadphoneMode));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsLocal>()->IsHeadphoneModeEnabled());
			
			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());
			Setting->AddEditCondition(MakeShared<FGameplaySettingWhenCondition>([](const ULocalPlayer*, FGameplaySettingEditableState& InOutEditableState)
			{
				if (!GetDefault<UGameplaySettingsLocal>()->CanModifyHeadphoneModeEnabled())
				{
					InOutEditableState.Kill(TEXT("Binaural Spatialization option cannot be modified on this platform."));
				}
			}));
			
			AudioDevicesSubCollection->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// HDR Audio Mode
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("HDRAudioMode"));
			Setting->SetDisplayName(LOCTEXT("HDRAudioMode_Name", "High Dynamic Range Audio"));
			Setting->SetDescriptionRichText(LOCTEXT("HDRAudioMode_Description",  "Enable high dynamic range audio. Changes the runtime processing chain to increase the dynamic range of the audio mixdown, appropriate for theater or more cinematic experiences."));
			
			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetHDRAudioMode));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetHDRAudioMode));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsLocal>()->GetHDRAudioMode());
			
			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());
			
			AudioDevicesSubCollection->AddSetting(Setting);
		}
	}
	//----------------------------------------------------------------------------------
	// Subtitles SubCollection
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* SubtitlesSubCollection = NewObject<UGameplaySettingCollection>();
		SubtitlesSubCollection->SetDevName(TEXT("SubtitlesSubCollection"));
		SubtitlesSubCollection->SetDisplayName(LOCTEXT("SubtitlesSubCollection_Name", "Subtitles"));
		Screen->AddSetting(SubtitlesSubCollection);
		
		//----------------------------------------------------------------------------------
		// Subtitle Page
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingCollectionPage* SubtitlePage = NewObject<UGameplaySettingCollectionPage>();
			SubtitlePage->SetDevName(TEXT("SubtitlePage"));
			SubtitlePage->SetDisplayName(LOCTEXT("SubtitlePage_Name", "Subtitles"));
			SubtitlePage->SetDescriptionRichText(LOCTEXT("SubtitlePage_Description", "Configure the visual appearance of subtitles."));
			SubtitlePage->SetNavigationText(LOCTEXT("SubtitlePage_Navigation", "Options"));

			SubtitlePage->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());

			SubtitlesSubCollection->AddSetting(SubtitlePage);
			
			//----------------------------------------------------------------------------------
			// Subtitles SubCollection
			//----------------------------------------------------------------------------------
			{
				UGameplaySettingCollection* SubtitlesSubSubCollection = NewObject<UGameplaySettingCollection>();
				SubtitlesSubSubCollection->SetDevName(TEXT("SubtitlesSubSubCollection"));
				SubtitlesSubSubCollection->SetDisplayName(LOCTEXT("SubtitlesSubSubCollection_Name", "Subtitles"));
				SubtitlePage->AddSetting(SubtitlesSubSubCollection);
				
				//----------------------------------------------------------------------------------
				// Subtitles
				//----------------------------------------------------------------------------------
				{
					UGameplaySettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Bool>();
					Setting->SetDevName(TEXT("Subtitles"));
					Setting->SetDisplayName(LOCTEXT("Subtitles_Name", "Subtitles"));
					Setting->SetDescriptionRichText(LOCTEXT("Subtitles_Description", "Turns subtitles on/off"));
				
					Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesEnabled));
					Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesEnabled));
					Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetSubtitlesEnabled());
				
					SubtitlesSubSubCollection->AddSetting(Setting);
				}
				//----------------------------------------------------------------------------------
				// Subtitles Text Size
				//----------------------------------------------------------------------------------
				{
					UGameplaySettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Enum>();
					Setting->SetDevName(TEXT("SubtitlesTextSize"));
					Setting->SetDisplayName(LOCTEXT("SubtitlesTextSize_Name", "Text Size"));
					Setting->SetDescriptionRichText(LOCTEXT("SubtitlesTextSize_Description", "Choose different sizes of the the subtitle text."));
				
					Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesTextSize));
					Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesTextSize));
					Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetSubtitlesTextSize());
					
					Setting->AddOption(EGameplaySubtitleDisplayTextSize::ExtraSmall, LOCTEXT("SubtitlesTextSize_ExtraSmall", "Extra Small"));
					Setting->AddOption(EGameplaySubtitleDisplayTextSize::Small, LOCTEXT("SubtitlesTextSize_Small", "Small"));
					Setting->AddOption(EGameplaySubtitleDisplayTextSize::Medium, LOCTEXT("SubtitlesTextSize_Medium", "Medium"));
					Setting->AddOption(EGameplaySubtitleDisplayTextSize::Large, LOCTEXT("SubtitlesTextSize_Large", "Large"));
					Setting->AddOption(EGameplaySubtitleDisplayTextSize::ExtraLarge, LOCTEXT("SubtitlesTextSize_ExtraLarge", "Extra Large"));
				
					SubtitlesSubSubCollection->AddSetting(Setting);
				}
				//----------------------------------------------------------------------------------
				// Subtitles Text Color
				//----------------------------------------------------------------------------------
				{
					UGameplaySettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Enum>();
					Setting->SetDevName(TEXT("SubtitlesTextColor"));
					Setting->SetDisplayName(LOCTEXT("SubtitlesTextColor_Name", "Text Color"));
					Setting->SetDescriptionRichText(LOCTEXT("SubtitlesTextColor_Description", "Choose different colors for the subtitle text."));
				
					Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesTextColor));
					Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesTextColor));
					Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetSubtitlesTextColor());
					
					Setting->AddOption(EGameplaySubtitleDisplayTextColor::White, LOCTEXT("SubtitlesTextColor_White", "White"));
					Setting->AddOption(EGameplaySubtitleDisplayTextColor::Black, LOCTEXT("SubtitlesTextColor_Black", "Black"));
					Setting->AddOption(EGameplaySubtitleDisplayTextColor::Red, LOCTEXT("SubtitlesTextColor_Red", "Red"));
					Setting->AddOption(EGameplaySubtitleDisplayTextColor::Green, LOCTEXT("SubtitlesTextColor_Green", "Green"));
					Setting->AddOption(EGameplaySubtitleDisplayTextColor::Blue, LOCTEXT("SubtitlesTextColor_Blue", "Blue"));
					Setting->AddOption(EGameplaySubtitleDisplayTextColor::Cyan, LOCTEXT("SubtitlesTextColor_Cyan", "Cyan"));
					Setting->AddOption(EGameplaySubtitleDisplayTextColor::Magenta, LOCTEXT("SubtitlesTextColor_Magenta", "Magenta"));
				
					SubtitlesSubSubCollection->AddSetting(Setting);
				}
				//----------------------------------------------------------------------------------
				// Subtitles Text Border
				//----------------------------------------------------------------------------------
				{
					UGameplaySettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Enum>();
					Setting->SetDevName(TEXT("SubtitlesTextBorder"));
					Setting->SetDisplayName(LOCTEXT("SubtitlesTextBorder_Name", "Text Border"));
					Setting->SetDescriptionRichText(LOCTEXT("SubtitlesTextBorder_Description", "Choose different borders for the subtitle text."));
				
					Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesTextBorder));
					Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesTextBorder));
					Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetSubtitlesTextBorder());
					
					Setting->AddOption(EGameplaySubtitleDisplayTextBorder::None, LOCTEXT("SubtitlesTextBorder_None", "None"));
					Setting->AddOption(EGameplaySubtitleDisplayTextBorder::Outline, LOCTEXT("SubtitlesTextBorder_Outline", "Outline"));
					Setting->AddOption(EGameplaySubtitleDisplayTextBorder::DropShadow, LOCTEXT("SubtitlesTextBorder_DropShadow", "DropShadow"));
				
					SubtitlesSubSubCollection->AddSetting(Setting);
				}
				//----------------------------------------------------------------------------------
				// Subtitles Background Opacity
				//----------------------------------------------------------------------------------
				{
					UGameplaySettingValueDiscreteDynamic_Enum* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Enum>();
					Setting->SetDevName(TEXT("SubtitlesBackgroundOpacity"));
					Setting->SetDisplayName(LOCTEXT("SubtitlesBackgroundOpacity_Name", "Background Opacity"));
					Setting->SetDescriptionRichText(LOCTEXT("SubtitlesBackgroundOpacity_Description", "Choose a different background or letterboxing for the subtitles."));
				
					Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetSubtitlesBackgroundOpacity));
					Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetSubtitlesBackgroundOpacity));
					Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetSubtitlesBackgroundOpacity());
					
					Setting->AddOption(EGameplaySubtitleDisplayBackgroundOpacity::Clear, LOCTEXT("SubtitlesBackgroundOpacity_Clear", "None"));
					Setting->AddOption(EGameplaySubtitleDisplayBackgroundOpacity::Low, LOCTEXT("SubtitlesBackgroundOpacity_Low", "Low"));
					Setting->AddOption(EGameplaySubtitleDisplayBackgroundOpacity::Medium, LOCTEXT("SubtitlesBackgroundOpacity_Medium", "Medium"));
					Setting->AddOption(EGameplaySubtitleDisplayBackgroundOpacity::High, LOCTEXT("SubtitlesBackgroundOpacity_High", "High"));
					Setting->AddOption(EGameplaySubtitleDisplayBackgroundOpacity::Solid, LOCTEXT("SubtitlesBackgroundOpacity_Solid", "Solid"));
				
					SubtitlesSubSubCollection->AddSetting(Setting);
				}
			}
		}
	}

	return Screen;
}

//============================================================================
// GAMEPLAY SETTINGS
//============================================================================

UGameplaySettingCollection* UGameplayExampleSettingRegistry::InitializeGameplaySettings(ULocalPlayer* InLocalPlayer)
{
	UGameplaySettingCollection* Screen = NewObject<UGameplaySettingCollection>();
	Screen->SetDevName(TEXT("GameplayCollection"));
	Screen->SetDisplayName(LOCTEXT("GameplayCollection_Name", "Gameplay"));
	Screen->Initialize(InLocalPlayer);

	//----------------------------------------------------------------------------------
	// Language SubCollection
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* LanguageSubCollection = NewObject<UGameplaySettingCollection>();
		LanguageSubCollection->SetDevName(TEXT("LanguageSubCollection"));
		LanguageSubCollection->SetDisplayName(LOCTEXT("LanguageSubCollection_Name", "Language"));
		Screen->AddSetting(LanguageSubCollection);

		//----------------------------------------------------------------------------------
		// Language Settings
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscrete_Language* Setting = NewObject<UGameplaySettingValueDiscrete_Language>();
			Setting->SetDevName(TEXT("Language"));
			Setting->SetDisplayName(LOCTEXT("Language_Name", "Language"));
			Setting->SetDescriptionRichText(LOCTEXT("Language_Description", "The language of the game."));

#if WITH_EDITOR
			if (GIsEditor)
			{
				Setting->SetDescriptionRichText(LOCTEXT("Language_DescriptionWithEditor", "The language of the game .\n\n<text color=\"#ffff00\">WARNING: Language changes will not affect PIE, you'll need to run with -game to test this, or change your PIE language options in the editor preferences.</>"));
			}
#endif

			Setting->AddEditCondition(FGameplaySettingWhenPlayingAsPrimaryPlayer::Get());

			LanguageSubCollection->AddSetting(Setting);
		}
	}

	return Screen;
}

//============================================================================
// MOUSE & KEYBOARD SETTINGS
//============================================================================

UGameplaySettingCollection* UGameplayExampleSettingRegistry::InitializeMouseAndKeyboardSettings(ULocalPlayer* InLocalPlayer)
{
	UGameplaySettingCollection* Screen = NewObject<UGameplaySettingCollection>();
	Screen->SetDevName(TEXT("KeyboardMouseCollection"));
	Screen->SetDisplayName(LOCTEXT("MouseKeyboardCollection_Name", "Keyboard & Mouse"));
	Screen->Initialize(InLocalPlayer);
	
	const TSharedRef<FGameplaySettingWhenCondition> WhenPlatformSupportsMouseAndKeyboard = MakeShared<FGameplaySettingWhenCondition>(
		[](const ULocalPlayer*, FGameplaySettingEditableState& InOutEditState)
		{
			const UCommonInputPlatformSettings* PlatformInput = UPlatformSettingsManager::Get().GetSettingsForPlatform<UCommonInputPlatformSettings>();
			if (!PlatformInput->SupportsInputType(ECommonInputType::MouseAndKeyboard))
			{
				InOutEditState.Kill(TEXT("Platform does not support mouse and keyboard."));
			}
		});

	//----------------------------------------------------------------------------------
	// Mouse Sensitivity
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* MouseSensitivity = NewObject<UGameplaySettingCollection>();
		MouseSensitivity->SetDevName(TEXT("MouseSensitivityCollection"));
		MouseSensitivity->SetDisplayName(LOCTEXT("MouseSensitivityCollection_Name", "Sensitivity"));
		Screen->AddSetting(MouseSensitivity);
		
		//----------------------------------------------------------------------------------
		// Mouse Sensitivity Yaw
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueScalarDynamic* Setting = NewObject<UGameplaySettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("MouseSensitivityYaw"));
			Setting->SetDisplayName(LOCTEXT("MouseSensitivityYaw_Name", "X-Axis Sensitivity"));
			Setting->SetDescriptionRichText(LOCTEXT("MouseSensitivityYaw_Description", "Sets the sensitivity of the mouse's horizontal (x) axis. With higher settings the camera will move faster when looking left and right with the mouse."));
			
			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetMouseSensitivityX));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetMouseSensitivityX));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetMouseSensitivityX());
			Setting->SetDisplayFormat(UGameplaySettingValueScalarDynamic::RawTwoDecimals);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.01);
			Setting->SetMinimumLimit(0.01);
			
			Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);
			
			MouseSensitivity->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Mouse Sensitivity Pitch
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueScalarDynamic* Setting = NewObject<UGameplaySettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("MouseSensitivityPitch"));
			Setting->SetDisplayName(LOCTEXT("MouseSensitivityPitch_Name", "Y-Axis Sensitivity"));
			Setting->SetDescriptionRichText(LOCTEXT("MouseSensitivityYaw_Description", "Sets the sensitivity of the mouse's vertical (y) axis. With higher settings the camera will move faster when looking left and right with the mouse."));
			
			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetMouseSensitivityY));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetMouseSensitivityY));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetMouseSensitivityY());
			Setting->SetDisplayFormat(UGameplaySettingValueScalarDynamic::RawTwoDecimals);
			Setting->SetSourceRangeAndStep(TRange<double>(0, 10), 0.01);
			Setting->SetMinimumLimit(0.01);
			
			Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);
			
			MouseSensitivity->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Invert Vertical Axis
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("InvertVerticalAxis"));
			Setting->SetDisplayName(LOCTEXT("InvertVerticalAxis_Name", "Invert Vertical Axis"));
			Setting->SetDescriptionRichText(LOCTEXT("InvertVerticalAxis_Description", "Enable the inversion of the vertical look axis."));
			
			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertVerticalAxis));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertVerticalAxis));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetInvertVerticalAxis());
			
			Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);
			
			MouseSensitivity->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Invert Horizontal Axis
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("InvertHorizontalAxis"));
			Setting->SetDisplayName(LOCTEXT("InvertHorizontalAxis_Name", "Invert Horizontal Axis"));
			Setting->SetDescriptionRichText(LOCTEXT("InvertHorizontalAxis_Description", "Enable the inversion of the vertical look axis."));
			
			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertHorizontalAxis));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertHorizontalAxis));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetInvertHorizontalAxis());
			
			Setting->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);
			
			MouseSensitivity->AddSetting(Setting);
		}
	}
	
	//----------------------------------------------------------------------------------
	// Bindings for Mouse & Keyboard - Automatically Generated
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* KeyBindingCollection = NewObject<UGameplaySettingCollection>();
		KeyBindingCollection->SetDevName(TEXT("KeyBindingCollection"));
		KeyBindingCollection->SetDisplayName(LOCTEXT("KeyBindingCollection_Name", "Keyboard & Mouse"));
		Screen->AddSetting(KeyBindingCollection);
		
		const UEnhancedInputLocalPlayerSubsystem* EnhancedInputSubsystem = InLocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
		const UEnhancedInputUserSettings* UserSettings = EnhancedInputSubsystem->GetUserSettings();
		
		// If you want to just get one profile pair, then you can do UserSettings->GetCurrentProfile
		
		// A map of key bindings mapped to their display category
		TMap<FString, UGameplaySettingCollection*> CategoryToSettingCollection;
		
		// Returns an existing setting collection for the display category if there is one.
		// If there isn't one, then it will create a new one and initialize it
		auto GetOrCreateSettingCollection = [&CategoryToSettingCollection, &Screen](FText DisplayCategory) -> UGameplaySettingCollection*
		{
			static const FString DefaultDevName = TEXT("Default_KBM");
			static const FText DefaultDevDisplayName = NSLOCTEXT("GameplayInputSettings", "GameplayInputDefaults", "Default Experiences");
			
			if (DisplayCategory.IsEmpty())
			{
				DisplayCategory = DefaultDevDisplayName;
			}
			
			const FString DisplayCategoryString = DisplayCategory.ToString();
			if (UGameplaySettingCollection** ExistingCategory = CategoryToSettingCollection.Find(DisplayCategoryString))
			{
				return *ExistingCategory;
			}
			
			UGameplaySettingCollection* ConfigSettingCollection = NewObject<UGameplaySettingCollection>();
			ConfigSettingCollection->SetDevName(FName(DisplayCategoryString));
			ConfigSettingCollection->SetDisplayName(DisplayCategory);
			Screen->AddSetting(ConfigSettingCollection);
			CategoryToSettingCollection.Add(DisplayCategoryString, ConfigSettingCollection);
			
			return ConfigSettingCollection;
		};
		
		static TSet<FName> CreatedMappingNames;
		CreatedMappingNames.Reset();
		
		for (const TPair<FString, TObjectPtr<UEnhancedPlayerMappableKeyProfile>>& ProfilePair : UserSettings->GetAllAvailableKeyProfiles())
		{
			const FString& ProfileName = ProfilePair.Key;
			const TObjectPtr<UEnhancedPlayerMappableKeyProfile>& Profile = ProfilePair.Value;
			
			for (const TPair<FName, FKeyMappingRow>& RowPair : Profile->GetPlayerMappingRows())
			{
				// Create a setting row for anything with valid mappings and that we haven't created yet
				if (RowPair.Value.HasAnyMappings())
				{
					// We only want keyboard keys on this settings screen, so we will filter down by mappings
					// that are set to keyboard keys
					FPlayerMappableKeyQueryOptions Options = {};
					Options.KeyToMatch = EKeys::W;
					Options.bMatchBasicKeyTypes = true;
					
					const FText& DesiredDisplayCategory = RowPair.Value.Mappings.begin()->GetDisplayCategory();
					if (UGameplaySettingCollection* Collection = GetOrCreateSettingCollection(DesiredDisplayCategory))
					{
						// Create the settings widget and initialize it, adding it to this config's section
						UGameplaySettingValueKeyboard* InputBinding = NewObject<UGameplaySettingValueKeyboard>();
						
						InputBinding->InitializeInputData(Profile, RowPair.Value, Options);
						InputBinding->AddEditCondition(WhenPlatformSupportsMouseAndKeyboard);
						
						Collection->AddSetting(InputBinding);
						CreatedMappingNames.Add(RowPair.Key);
					}
					else
					{
						ensure(false);
					}
				}
			}
		}
	}

	return Screen;
}

//============================================================================
// GAMEPAD SETTINGS
//============================================================================

UGameplaySettingCollection* UGameplayExampleSettingRegistry::InitializeGamepadSettings(ULocalPlayer* InLocalPlayer)
{
	UGameplaySettingCollection* Screen = NewObject<UGameplaySettingCollection>();
	Screen->SetDevName(TEXT("GamepadCollection"));
	Screen->SetDisplayName(LOCTEXT("GamepadCollection_Name", "Gamepad"));
	Screen->Initialize(InLocalPlayer);

	//----------------------------------------------------------------------------------
	// Hardware
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* HardwareCollection = NewObject<UGameplaySettingCollection>();
		HardwareCollection->SetDevName(TEXT("HardwareCollection"));
		HardwareCollection->SetDisplayName(LOCTEXT("HardwareCollection_Name", "Hardware"));
		Screen->AddSetting(HardwareCollection);
		
		//----------------------------------------------------------------------------------
		// Controller Hardware
		//----------------------------------------------------------------------------------
		/*{
			UGameplaySettingValueDiscreteDynamic* Setting = NewObject<UGameplaySettingValueDiscreteDynamic>();
			Setting->SetDevName(TEXT("ControllerHardware"));
			Setting->SetDisplayName(LOCTEXT("ControllerHardware_Name", "Controller Hardware"));
			Setting->SetDescriptionRichText(LOCTEXT("ControllerHardware_Description", "The type of controller you're using."));
			Setting->SetDynamicGetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(GetControllerPlatform));
			Setting->SetDynamicSetter(GET_LOCAL_SETTINGS_FUNCTION_PATH(SetControllerPlatform));
			
			if (const UCommonInputPlatformSettings* PlatformInputSettings = UPlatformSettingsManager::Get().GetSettingsForPlatform<UCommonInputPlatformSettings>())
			{
				const TArray<TSoftClassPtr<UCommonInputBaseControllerData>>& ControllerDatas = PlatformInputSettings->GetControllerData();
				for (TSoftClassPtr<UCommonInputBaseControllerData> ControllerDataPtr : ControllerDatas)
				{
					if (TSubclassOf<UCommonInputBaseControllerData> ControllerDataClass = ControllerDataPtr.LoadSynchronous())
					{
						const UCommonInputBaseControllerData* ControllerData = ControllerDataClass.GetDefaultObject();
						if (ControllerData->InputType == ECommonInputType::Gamepad)
						{
							Setting->AddDynamicOption(ControllerData->GamepadName.ToString(), ControllerData->GamepadDisplayName);
						}
					}
				}

				// Add the setting if we can select more than one game controller type on this platform,
				// and we are allowed to change it
				if (Setting->GetDynamicOptions().Num() > 1 && PlatformInputSettings->CanChangeGamepadType())
				{
					HardwareCollection->AddSetting(Setting);

					const FName CurrentControllerPlatform = GetDefault<UGameplaySettingsLocal>()->GetControllerPlatform();
					if (CurrentControllerPlatform == NAME_None)
					{
						Setting->SetDiscreteOptionByIndex(0);
					}
					else
					{
						Setting->SetDefaultValueFromString(CurrentControllerPlatform.ToString());
					}
				}
			}
		}*/
		//----------------------------------------------------------------------------------
		// Gamepad Vibration
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("GamepadVibration"));
			Setting->SetDisplayName(LOCTEXT("GamepadVibration_Name", "Vibration"));
			Setting->SetDescriptionRichText(LOCTEXT("GamepadVibration_Description", "Turns controller vibration on/off."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetForceFeedbackEnabled));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetForceFeedbackEnabled));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetForceFeedbackEnabled());

			HardwareCollection->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Invert Vertical Axis
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("GamepadInvertVerticalAxis"));
			Setting->SetDisplayName(LOCTEXT("GamepadInvertVerticalAxis_Name", "Invert Vertical Axis"));
			Setting->SetDescriptionRichText(LOCTEXT("GamepadInvertVerticalAxis_Description", "Enable the inversion of the vertical look axis."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertVerticalAxis));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertVerticalAxis));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetInvertVerticalAxis());

			HardwareCollection->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Invert Horizontal Axis
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueDiscreteDynamic_Bool* Setting = NewObject<UGameplaySettingValueDiscreteDynamic_Bool>();
			Setting->SetDevName(TEXT("GamepadInvertHorizontalAxis"));
			Setting->SetDisplayName(LOCTEXT("GamepadInvertHorizontalAxis_Name", "Invert Horizontal Axis"));
			Setting->SetDescriptionRichText(LOCTEXT("GamepadInvertHorizontalAxis_Description", "Enable the inversion of the horizontal look axis."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetInvertHorizontalAxis));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetInvertHorizontalAxis));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetInvertHorizontalAxis());

			HardwareCollection->AddSetting(Setting);
		}
	}
	//----------------------------------------------------------------------------------
	// Gamepad Binding
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* GamepadBindingCollection = NewObject<UGameplaySettingCollection>();
		GamepadBindingCollection->SetDevName(TEXT("GamepadBindingCollection"));
		GamepadBindingCollection->SetDisplayName(LOCTEXT("GamepadBindingCollection_Name", "Controls"));
		Screen->AddSetting(GamepadBindingCollection);
	}
	//----------------------------------------------------------------------------------
	// Gamepad Deadzone
	//----------------------------------------------------------------------------------
	{
		UGameplaySettingCollection* GamepadDeadzoneCollection = NewObject<UGameplaySettingCollection>();
		GamepadDeadzoneCollection->SetDevName(TEXT("GamepadDeadzoneCollection"));
		GamepadDeadzoneCollection->SetDisplayName(LOCTEXT("GamepadDeadzoneCollection_Name", "Deadzone"));
		Screen->AddSetting(GamepadDeadzoneCollection);
		
		//----------------------------------------------------------------------------------
		// Move Stick Deadzone
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueScalarDynamic* Setting = NewObject<UGameplaySettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("MoveStickDeadZone"));
			Setting->SetDisplayName(LOCTEXT("MoveStickDeadZone_Name", "Left Stick DeadZone"));
			Setting->SetDescriptionRichText(LOCTEXT("MoveStickDeadZone_Description", "Increase or decrease the area surrounding the stick that we ignore input from.  Setting this value too low may result in the character continuing to move even after removing your finger from the stick."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadMoveStickDeadZone));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadMoveStickDeadZone));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetGamepadMoveStickDeadZone());
			Setting->SetDisplayFormat(UGameplaySettingValueScalarDynamic::ZeroToOnePercent);
			Setting->SetMinimumLimit(0.05);
			Setting->SetMaximumLimit(0.95);

			GamepadDeadzoneCollection->AddSetting(Setting);
		}
		//----------------------------------------------------------------------------------
		// Look Stick Deadzone
		//----------------------------------------------------------------------------------
		{
			UGameplaySettingValueScalarDynamic* Setting = NewObject<UGameplaySettingValueScalarDynamic>();
			Setting->SetDevName(TEXT("LookStickDeadZone"));
			Setting->SetDisplayName(LOCTEXT("LookStickDeadZone_Name", "Left Stick DeadZone"));
			Setting->SetDescriptionRichText(LOCTEXT("LookStickDeadZone_Description", "Increase or decrease the area surrounding the stick that we ignore input from.  Setting this value too low may result in the camera continuing to move even after removing your finger from the stick."));

			Setting->SetDynamicGetter(GET_SHARED_SETTINGS_FUNCTION_PATH(GetGamepadLookStickDeadZone));
			Setting->SetDynamicSetter(GET_SHARED_SETTINGS_FUNCTION_PATH(SetGamepadLookStickDeadZone));
			Setting->SetDefaultValue(GetDefault<UGameplaySettingsShared>()->GetGamepadLookStickDeadZone());
			Setting->SetDisplayFormat(UGameplaySettingValueScalarDynamic::ZeroToOnePercent);
			Setting->SetMinimumLimit(0.05);
			Setting->SetMaximumLimit(0.95);

			GamepadDeadzoneCollection->AddSetting(Setting);
		}
	}

	return Screen;
}

#undef LOCTEXT_NAMESPACE
