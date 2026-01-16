// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingValueDiscrete.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/GameplayCommonPerformanceSettings.h"
#include "Framework/GameplaySettingsLocal.h"
#include "Framework/GameplaySettingsShared.h"
#include "GenericPlatform/GenericApplication.h"
#include "GameFramework/GameUserSettings.h"
#include "EditCondition/GameplaySettingEditCondition.h"
#include "UnrealEngine.h"
#include "Engine/LocalPlayer.h"
#include "RHI.h"
#include "CommonUIVisibilitySubsystem.h"
#include "GameplayTagContainer.h"
#include "Interfaces/GameplayCommonSettingsInterface.h"
#include "Internationalization/Culture.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingValueDiscrete)

#define LOCTEXT_NAMESPACE "SettingValueDiscrete"

static constexpr int32 SettingSystemDefaultLanguageIndex = 0;

// ===================================================
// UGameplaySettingValueDiscrete
// ===================================================
UGameplaySettingValueDiscrete::UGameplaySettingValueDiscrete(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

FString UGameplaySettingValueDiscrete::GetAnalyticsValue() const
{
	const TArray<FText> Options = GetDiscreteOptions();
	const int32 CurrentOptionIndex = GetDiscreteOptionIndex();
	if (Options.IsValidIndex(CurrentOptionIndex))
	{
		if (const FString* SourceString = FTextInspector::GetSourceString(Options[CurrentOptionIndex]))
		{
			return *SourceString;
		}
	}

	return TEXT("<Unknown Index>");
}

// ===================================================
// UGameplaySettingValueDiscrete_Resolution
// ===================================================
UGameplaySettingValueDiscrete_Resolution::UGameplaySettingValueDiscrete_Resolution(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UGameplaySettingValueDiscrete_Resolution::OnInitialized()
{
	Super::OnInitialized();

	InitializeResolutions();
}

void UGameplaySettingValueDiscrete_Resolution::StoreInitial()
{
	// Ignored
}

void UGameplaySettingValueDiscrete_Resolution::ResetToDefault()
{
	// Ignored
}

void UGameplaySettingValueDiscrete_Resolution::RestoreToInitial()
{
	// Ignored
}

void UGameplaySettingValueDiscrete_Resolution::SetDiscreteOptionByIndex(int32 Index)
{
	if (Resolutions.IsValidIndex(Index) && Resolutions[Index].IsValid())
	{
		GEngine->GetGameUserSettings()->SetScreenResolution(Resolutions[Index]->GetResolution());
		NotifySettingChanged(EGameplaySettingChangeReason::Change);
	}
}

int32 UGameplaySettingValueDiscrete_Resolution::GetDiscreteOptionIndex() const
{
	const UGameUserSettings* UserSettings = CastChecked<const UGameUserSettings>(GEngine->GetGameUserSettings());
	return FindIndexOfDisplayResolutionForceValid(UserSettings->GetScreenResolution());
}

TArray<FText> UGameplaySettingValueDiscrete_Resolution::GetDiscreteOptions() const
{
	TArray<FText> ReturnResolutionTexts;

	for (int32 i = 0; i < Resolutions.Num(); ++i)
	{
		ReturnResolutionTexts.Add(Resolutions[i]->GetDisplayText());
	}

	return ReturnResolutionTexts;
}

void UGameplaySettingValueDiscrete_Resolution::OnDependencyChanged()
{
	const FIntPoint CurrentResolution = GEngine->GetGameUserSettings()->GetScreenResolution();
	SelectAppropriateResolutions();
	SetDiscreteOptionByIndex(FindClosestResolutionIndex(CurrentResolution));
}

void UGameplaySettingValueDiscrete_Resolution::InitializeResolutions()
{
	Resolutions.Empty();
	ResolutionsFullscreen.Empty();
	ResolutionsWindowed.Empty();
	ResolutionsWindowedFullscreen.Empty();

	FDisplayMetrics InitialDisplayMetrics;
	FSlateApplication::Get().GetInitialDisplayMetrics(InitialDisplayMetrics);

	FScreenResolutionArray ResArray;
	RHIGetAvailableResolutions(ResArray, true);

	// Determine available windowed modes
	{
		TArray<FIntPoint> WindowedResolutions;
		const FIntPoint MinResolution(1280, 720);
		// Use the primary display resolution minus 1 to exclude the primary display resolution from the list.
		// This is so you don't make a window so large that part of the game is off screen and you are unable to change resolutions back.
		const FIntPoint MaxResolution(InitialDisplayMetrics.PrimaryDisplayWidth - 1, InitialDisplayMetrics.PrimaryDisplayHeight - 1);
		// Excluding 4:3 and below
		constexpr float MinAspectRatio = 16 / 10.f;

		if (MaxResolution.X >= MinResolution.X && MaxResolution.Y >= MinResolution.Y)
		{
			GetStandardWindowResolutions(MinResolution, MaxResolution, MinAspectRatio, WindowedResolutions);
		}

		if (GSystemResolution.WindowMode == EWindowMode::Windowed)
		{
			WindowedResolutions.AddUnique(FIntPoint(GSystemResolution.ResX, GSystemResolution.ResY));
			WindowedResolutions.Sort([](const FIntPoint& A, const FIntPoint& B) { return A.X != B.X ? A.X < B.X : A.Y < B.Y; });
		}

		// If there were no standard resolutions. Add the primary display size, just so one exists.
		// This might happen if we are running on a non-standard device.
		if (WindowedResolutions.Num() == 0)
		{
			WindowedResolutions.Add(FIntPoint(InitialDisplayMetrics.PrimaryDisplayWidth, InitialDisplayMetrics.PrimaryDisplayHeight));
		}

		ResolutionsWindowed.Empty(WindowedResolutions.Num());
		for (const FIntPoint& Res : WindowedResolutions)
		{
			TSharedRef<FScreenResolutionEntry> Entry = MakeShared<FScreenResolutionEntry>();
			Entry->Width = Res.X;
			Entry->Height = Res.Y;

			ResolutionsWindowed.Add(Entry);
		}
	}

	// Determine available windowed full-screen modes
	{
		const FScreenResolutionRHI* RHIInitialResolution = ResArray.FindByPredicate([InitialDisplayMetrics](const FScreenResolutionRHI& ScreenRes) {
			return ScreenRes.Width == InitialDisplayMetrics.PrimaryDisplayWidth && ScreenRes.Height == InitialDisplayMetrics.PrimaryDisplayHeight;
		});

		const TSharedRef<FScreenResolutionEntry> Entry = MakeShared<FScreenResolutionEntry>();
		if (RHIInitialResolution)
		{
			// If this is in the official list use that
			Entry->Width = RHIInitialResolution->Width;
			Entry->Height = RHIInitialResolution->Height;
			Entry->RefreshRate = RHIInitialResolution->RefreshRate;
		}
		else
		{
			// Custom resolution the RHI doesn't expect
			Entry->Width = InitialDisplayMetrics.PrimaryDisplayWidth;
			Entry->Height = InitialDisplayMetrics.PrimaryDisplayHeight;

			// TODO: Unsure how to calculate refresh rate
			Entry->RefreshRate = FPlatformMisc::GetMaxRefreshRate();
		}

		ResolutionsWindowedFullscreen.Add(Entry);
	}

	// Determine available full-screen modes
	if (ResArray.Num() > 0)
	{
		// try more strict first then more relaxed, we want at least one resolution to remain
		for (int32 FilterThreshold = 0; FilterThreshold < 3; ++FilterThreshold)
		{
			for (int32 ModeIndex = 0; ModeIndex < ResArray.Num(); ModeIndex++)
			{
				const FScreenResolutionRHI& ScreenRes = ResArray[ModeIndex];

				// first try with struct test, than relaxed test
				if (ShouldAllowFullscreenResolution(ScreenRes, FilterThreshold))
				{
					TSharedRef<FScreenResolutionEntry> Entry = MakeShared<FScreenResolutionEntry>();
					Entry->Width = ScreenRes.Width;
					Entry->Height = ScreenRes.Height;
					Entry->RefreshRate = ScreenRes.RefreshRate;

					ResolutionsFullscreen.Add(Entry);
				}
			}

			if (ResolutionsFullscreen.Num())
			{
				// we found some resolutions, otherwise we try with more relaxed tests
				break;
			}
		}
	}

	SelectAppropriateResolutions();
}

void UGameplaySettingValueDiscrete_Resolution::SelectAppropriateResolutions()
{
	EWindowMode::Type const WindowMode = GEngine->GetGameUserSettings()->GetFullscreenMode();
	if (LastWindowMode != WindowMode)
	{
		LastWindowMode = WindowMode;
		Resolutions.Empty();
		
		switch (WindowMode)
		{
		case EWindowMode::Windowed:
			Resolutions.Append(ResolutionsWindowed);
			break;
		case EWindowMode::WindowedFullscreen:
			Resolutions.Append(ResolutionsWindowedFullscreen);
			break;
		case EWindowMode::Fullscreen:
			Resolutions.Append(ResolutionsFullscreen);
			break;
		}
		
		NotifyEditConditionsChanged();
	}
}

// To filter out odd resolution so UI and testing has less issues. This is game specific.
// @param ScreenRes resolution and
// @param FilterThreshold 0/1/2 to make sure we get at least some resolutions (might be an issues with UI but at least we get some resolution entries)
bool UGameplaySettingValueDiscrete_Resolution::ShouldAllowFullscreenResolution(const FScreenResolutionRHI& SrcScreenRes, int32 FilterThreshold) const
{
	FScreenResolutionRHI ScreenRes = SrcScreenRes;

	// expected: 4:3=1.333, 16:9=1.777, 16:10=1.6, multi-monitor-wide: >2
	const bool bIsPortrait = ScreenRes.Width < ScreenRes.Height;
	float AspectRatio = (float)ScreenRes.Width / (float)ScreenRes.Height;

	// If portrait, flip values back to landscape so we can don't have to special case all the tests below
	if (bIsPortrait)
	{
		AspectRatio = 1.0f / AspectRatio;
		ScreenRes.Width = SrcScreenRes.Height;
		ScreenRes.Height = SrcScreenRes.Width;
	}

	// Filter out resolutions that don't match the native aspect ratio of the primary monitor
	// TODO: Other games allow the user to choose which monitor the games goes fullscreen on. This would allow
	// this filtering to be correct when the users monitors are of different types! ATM, the game can change
	// which monitor it uses based on other factors (max window overlap etc.) so we could end up choosing a
	// resolution which the target monitor doesn't support.
	if (FilterThreshold < 1)
	{
		FDisplayMetrics DisplayMetrics;
		FSlateApplication::Get().GetInitialDisplayMetrics(DisplayMetrics);

		// Default display aspect to required aspect in case this platform can't provide the information. Forces acceptance of this resolution.
		float DisplayAspect = AspectRatio;

		// Some platforms might not be able to detect the native resolution of the display device, so don't filter in that case
		for (int32 MonitorIndex = 0; MonitorIndex < DisplayMetrics.MonitorInfo.Num(); ++MonitorIndex)
		{
			const FMonitorInfo& MonitorInfo = DisplayMetrics.MonitorInfo[MonitorIndex];
			if (MonitorInfo.bIsPrimary)
			{
				DisplayAspect = (float)MonitorInfo.NativeWidth / (float)MonitorInfo.NativeHeight;
				break;
			}
		}

		// If aspects are not almost exactly equal, reject
		if (FMath::Abs(DisplayAspect - AspectRatio) > KINDA_SMALL_NUMBER)
		{
			return false;
		}
	}

	// more relaxed tests have a larger FilterThreshold

	// minimum is 1280x720
	if (FilterThreshold < 2 && (ScreenRes.Width < 1280 || ScreenRes.Height < 720))
	{
		// filter resolutions that are too small
		return false;
	}

	return true;
}

int32 UGameplaySettingValueDiscrete_Resolution::FindIndexOfDisplayResolution(const FIntPoint& InPoint) const
{
	// find the current res
	for (int32 i = 0, Num = Resolutions.Num(); i < Num; ++i)
	{
		if (Resolutions[i]->GetResolution() == InPoint)
		{
			return i;
		}
	}

	return INDEX_NONE;
}

int32 UGameplaySettingValueDiscrete_Resolution::FindIndexOfDisplayResolutionForceValid(const FIntPoint& InPoint) const
{
	int32 Result = FindIndexOfDisplayResolution(InPoint);
	if (Result == INDEX_NONE && Resolutions.Num() > 0)
	{
		Result = Resolutions.Num() - 1;
	}

	return Result;
}

int32 UGameplaySettingValueDiscrete_Resolution::FindClosestResolutionIndex(const FIntPoint& Resolution) const
{
	int32 Index = 0;
	int32 LastDiff = Resolution.SizeSquared();

	for (int32 i = 0, Num = Resolutions.Num(); i < Num; ++i)
	{
		// We compare the squared diagonals
		const int32 Diff = FMath::Abs(Resolution.SizeSquared() - Resolutions[i]->GetResolution().SizeSquared());
		if (Diff <= LastDiff)
		{				
			Index = i;
		}
		LastDiff = Diff;
	}

	return Index;
}

void UGameplaySettingValueDiscrete_Resolution::GetStandardWindowResolutions(const FIntPoint& MinResolution, const FIntPoint& MaxResolution, float MinAspectRatio, TArray<FIntPoint>& OutResolutions)
{
	static TArray<FIntPoint> StandardResolutions;
	if (StandardResolutions.Num() == 0)
	{
		// Standard resolutions as provided by Wikipedia (http://en.wikipedia.org/wiki/Graphics_display_resolution)

		// Extended Graphics Array
		{
			new(StandardResolutions) FIntPoint(1024, 768); // XGA

														   // WXGA (3 versions)
			new(StandardResolutions) FIntPoint(1366, 768);	// FWXGA
			new(StandardResolutions) FIntPoint(1360, 768);
			new(StandardResolutions) FIntPoint(1280, 800);

			new(StandardResolutions) FIntPoint(1152, 864);	// XGA+
			new(StandardResolutions) FIntPoint(1440, 900);	// WXGA+
			new(StandardResolutions) FIntPoint(1280, 1024); // SXGA
			new(StandardResolutions) FIntPoint(1400, 1050); // SXGA+
			new(StandardResolutions) FIntPoint(1680, 1050); // WSXGA+
			new(StandardResolutions) FIntPoint(1600, 1200); // UXGA
			new(StandardResolutions) FIntPoint(1920, 1200); // WUXGA
		}

		// Quad Extended Graphics Array
		{
			new(StandardResolutions) FIntPoint(2048, 1152); // QWXGA
			new(StandardResolutions) FIntPoint(2048, 1536); // QXGA
			new(StandardResolutions) FIntPoint(2560, 1600); // WQXGA
			new(StandardResolutions) FIntPoint(2560, 2048); // QSXGA
			new(StandardResolutions) FIntPoint(3200, 2048); // WQSXGA
			new(StandardResolutions) FIntPoint(3200, 2400); // QUXGA
			new(StandardResolutions) FIntPoint(3840, 2400); // WQUXGA
		}

		// Hyper Extended Graphics Array
		{
			new(StandardResolutions) FIntPoint(4096, 3072); // HXGA
			new(StandardResolutions) FIntPoint(5120, 3200); // WHXGA
			new(StandardResolutions) FIntPoint(5120, 4096); // HSXGA
			new(StandardResolutions) FIntPoint(6400, 4096); // WHSXGA
			new(StandardResolutions) FIntPoint(6400, 4800); // HUXGA
			new(StandardResolutions) FIntPoint(7680, 4800); // WHUXGA
		}

		// High-Definition
		{
			new(StandardResolutions) FIntPoint(640, 360);	// nHD
			new(StandardResolutions) FIntPoint(960, 540);	// qHD
			new(StandardResolutions) FIntPoint(1280, 720);	// HD
			new(StandardResolutions) FIntPoint(1920, 1080); // FHD
			new(StandardResolutions) FIntPoint(2560, 1440); // QHD
			new(StandardResolutions) FIntPoint(3200, 1800); // WQXGA+
			new(StandardResolutions) FIntPoint(3840, 2160); // UHD 4K
			new(StandardResolutions) FIntPoint(4096, 2160); // Digital Cinema Initiatives 4K
			new(StandardResolutions) FIntPoint(7680, 4320); // FUHD
			new(StandardResolutions) FIntPoint(5120, 2160); // UHD 5K
			new(StandardResolutions) FIntPoint(5120, 2880); // UHD+
			new(StandardResolutions) FIntPoint(15360, 8640);// QUHD
		}

		// Sort the list by total resolution size
		StandardResolutions.Sort([](const FIntPoint& A, const FIntPoint& B) { return (A.X * A.Y) < (B.X * B.Y); });
	}

	// Return all standard resolutions that are within the size constraints
	for (const auto& Resolution : StandardResolutions)
	{
		if (Resolution.X >= MinResolution.X && Resolution.Y >= MinResolution.Y && Resolution.X <= MaxResolution.X && Resolution.Y <= MaxResolution.Y)
		{
			const float AspectRatio = Resolution.X / (float)Resolution.Y;
			if (AspectRatio > MinAspectRatio || FMath::IsNearlyEqual(AspectRatio, MinAspectRatio))
			{
				OutResolutions.Add(Resolution);
			}
		}
	}
}

FText UGameplaySettingValueDiscrete_Resolution::FScreenResolutionEntry::GetDisplayText() const
{
	if (!OverrideText.IsEmpty())
	{
		return OverrideText;
	}

	FText Aspect = FText::GetEmpty();

	// expected: 4:3=1.333, 16:9=1.777, 16:10=1.6, multi-monitor-wide: >2
	float AspectRatio = (float)Width / (float)Height;

	if (FMath::Abs(AspectRatio - (4.0f / 3.0f)) < KINDA_SMALL_NUMBER)
	{
		Aspect = LOCTEXT("AspectRatio-4:3", "4:3");
	}
	else if (FMath::Abs(AspectRatio - (16.0f / 9.0f)) < KINDA_SMALL_NUMBER)
	{
		Aspect = LOCTEXT("AspectRatio-16:9", "16:9");
	}
	else if (FMath::Abs(AspectRatio - (16.0f / 10.0f)) < KINDA_SMALL_NUMBER)
	{
		Aspect = LOCTEXT("AspectRatio-16:10", "16:10");
	}
	else if (FMath::Abs(AspectRatio - (3.0f / 4.0f)) < KINDA_SMALL_NUMBER)
	{
		Aspect = LOCTEXT("AspectRatio-3:4", "3:4");
	}
	else if (FMath::Abs(AspectRatio - (9.0f / 16.0f)) < KINDA_SMALL_NUMBER)
	{
		Aspect = LOCTEXT("AspectRatio-9:16", "9:16");
	}
	else if (FMath::Abs(AspectRatio - (10.0f / 16.0f)) < KINDA_SMALL_NUMBER)
	{
		Aspect = LOCTEXT("AspectRatio-10:16", "10:16");
	}

	FNumberFormattingOptions Options;
	Options.UseGrouping = false;

	FFormatNamedArguments Args;
	Args.Add(TEXT("X"), FText::AsNumber(Width, &Options));
	Args.Add(TEXT("Y"), FText::AsNumber(Height, &Options));
	Args.Add(TEXT("AspectRatio"), Aspect);
	Args.Add(TEXT("RefreshRate"), RefreshRate);

	return FText::Format(LOCTEXT("AspectRatio", "{X} x {Y}"), Args);
}

// ===================================================
// UGameplaySettingValueDiscrete_OverallQuality
// ===================================================

UGameplaySettingValueDiscrete_OverallQuality::UGameplaySettingValueDiscrete_OverallQuality(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

void UGameplaySettingValueDiscrete_OverallQuality::OnInitialized()
{
	Super::OnInitialized();

	UGameplaySettingsLocal* UserSettings = UGameplaySettingsLocal::Get();
	const int32 MaxQualityLevel = UserSettings->GetMaxSupportedOverallQualityLevel();

	auto AddOptionIfPossible = [&](int Index, FText&& Value)
	{
		if ((MaxQualityLevel < 0) || (Index <= MaxQualityLevel))
		{
			Options.Add(Value);
		}
	};

	AddOptionIfPossible(0, LOCTEXT("VideoQualityOverall_Low", "Low"));
	AddOptionIfPossible(1, LOCTEXT("VideoQualityOverall_Medium", "Medium"));
	AddOptionIfPossible(2, LOCTEXT("VideoQualityOverall_High", "High"));
	AddOptionIfPossible(3, LOCTEXT("VideoQualityOverall_Epic", "Epic"));

	OptionsWithCustom = Options;
	OptionsWithCustom.Add(LOCTEXT("VideoQualityOverall_Custom", "Custom"));

	/*const int32 LowestQualityWithFrameRateLimit = UserSettings->GetLowestQualityWithFrameRateLimit();
	if (Options.IsValidIndex(LowestQualityWithFrameRateLimit))
	{
		SetWarningRichText(FText::Format(LOCTEXT("OverallQuality_Mobile_ImpactsFramerate", "<strong>Note: Changing the Quality setting to {0} or higher might limit your framerate.</>"), Options[LowestQualityWithFrameRateLimit]));
	}*/
}

void UGameplaySettingValueDiscrete_OverallQuality::StoreInitial()
{
	// Ignored
}

void UGameplaySettingValueDiscrete_OverallQuality::ResetToDefault()
{
	// Ignored
}

void UGameplaySettingValueDiscrete_OverallQuality::RestoreToInitial()
{
	// Ignored
}

void UGameplaySettingValueDiscrete_OverallQuality::SetDiscreteOptionByIndex(int32 Index)
{
	UGameUserSettings* UserSettings = CastChecked<UGameUserSettings>(GEngine->GetGameUserSettings());

	if (Index == GetCustomOptionIndex())
	{
		// Leave everything as is we're in a custom setup.
	}
	else
	{
		// Low / Medium / High / Epic
		UserSettings->SetOverallScalabilityLevel(Index);
	}

	NotifySettingChanged(EGameplaySettingChangeReason::Change);
}

int32 UGameplaySettingValueDiscrete_OverallQuality::GetDiscreteOptionIndex() const
{
	const int32 OverallQualityLevel = GetOverallQualityLevel();
	if (OverallQualityLevel == INDEX_NONE)
	{
		return GetCustomOptionIndex();
	}

	return OverallQualityLevel;
}

TArray<FText> UGameplaySettingValueDiscrete_OverallQuality::GetDiscreteOptions() const
{
	const int32 OverallQualityLevel = GetOverallQualityLevel();
	if (OverallQualityLevel == INDEX_NONE)
	{
		return OptionsWithCustom;
	}

	return Options;
}

int32 UGameplaySettingValueDiscrete_OverallQuality::GetCustomOptionIndex() const
{
	return OptionsWithCustom.Num() - 1;
}

int32 UGameplaySettingValueDiscrete_OverallQuality::GetOverallQualityLevel() const
{
	const UGameUserSettings* UserSettings = CastChecked<const UGameUserSettings>(GEngine->GetGameUserSettings());
	return UserSettings->GetOverallScalabilityLevel();
}

class FGameplaySettingEditCondition_PerfStatAllowed : public FGameplaySettingEditCondition
{
public:
	static TSharedRef<FGameplaySettingEditCondition_PerfStatAllowed> Get(EGameplayDisplayablePerformanceStat Stat)
	{
		return MakeShared<FGameplaySettingEditCondition_PerfStatAllowed>(Stat);
	}

	FGameplaySettingEditCondition_PerfStatAllowed(EGameplayDisplayablePerformanceStat Stat)
		: AssociatedStat(Stat)
	{
	}

	//~Begin FGameplaySettingEditCondition interface
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState) const override
	{
		const FGameplayTagContainer& VisibilityTags = UCommonUIVisibilitySubsystem::GetChecked(InLocalPlayer)->GetVisibilityTags();

		bool bCanShowStat = false;
		for (const FGameplayPerformanceStatGroup& Group : GetDefault<UGameplayCommonPerformanceSettings>()->UserFacingPerformanceStats) //@TODO: Move this stuff to per-platform instead of doing vis queries too?
		{
			if (Group.AllowedStats.Contains(AssociatedStat))
			{
				const bool bShowGroup = (Group.VisibilityQuery.IsEmpty() || Group.VisibilityQuery.Matches(VisibilityTags));
				if (bShowGroup)
				{
					bCanShowStat = true;
					break;
				}
			}
		}

		if (!bCanShowStat)
		{
			InOutEditState.Hide(TEXT("Stat is not listed in UGameplayPlatformSpecificRenderingSettings or is suppressed by current platform traits"));
		}
	}
	//~End of FGameplaySettingEditCondition interface

private:
	EGameplayDisplayablePerformanceStat AssociatedStat;
};

// ===================================================
// UGameplaySettingValueDiscrete_PerfStat
// ===================================================

UGameplaySettingValueDiscrete_PerfStat::UGameplaySettingValueDiscrete_PerfStat()
{

}

void UGameplaySettingValueDiscrete_PerfStat::SetStat(EGameplayDisplayablePerformanceStat InStat)
{
	StatToDisplay = InStat;
	SetDevName(FName(*FString::Printf(TEXT("PerfStat_%d"), (int32)StatToDisplay)));
	AddEditCondition(FGameplaySettingEditCondition_PerfStatAllowed::Get(StatToDisplay));
}

void UGameplaySettingValueDiscrete_PerfStat::AddMode(FText&& Label, EGameplayPerformanceStatDisplayMode Mode)
{
	Options.Emplace(MoveTemp(Label));
	DisplayModes.Add(Mode);
}

void UGameplaySettingValueDiscrete_PerfStat::OnInitialized()
{
	Super::OnInitialized();

	AddMode(LOCTEXT("PerfStatDisplayMode_None", "None"), EGameplayPerformanceStatDisplayMode::Hidden);
	AddMode(LOCTEXT("PerfStatDisplayMode_TextOnly", "Text Only"), EGameplayPerformanceStatDisplayMode::TextOnly);
	AddMode(LOCTEXT("PerfStatDisplayMode_GraphOnly", "Graph Only"), EGameplayPerformanceStatDisplayMode::GraphOnly);
	AddMode(LOCTEXT("PerfStatDisplayMode_TextAndGraph", "Text & Graph"), EGameplayPerformanceStatDisplayMode::TextAndGraph);
}

void UGameplaySettingValueDiscrete_PerfStat::StoreInitial()
{
	const UGameplaySettingsLocal* Settings = UGameplaySettingsLocal::Get();
	InitialMode = Settings->GetPerfStatDisplayState(StatToDisplay);
}

void UGameplaySettingValueDiscrete_PerfStat::ResetToDefault()
{
	UGameplaySettingsLocal* Settings = UGameplaySettingsLocal::Get();
	Settings->SetPerfStatDisplayState(StatToDisplay, EGameplayPerformanceStatDisplayMode::Hidden);
	NotifySettingChanged(EGameplaySettingChangeReason::ResetToDefault);
}

void UGameplaySettingValueDiscrete_PerfStat::RestoreToInitial()
{
	UGameplaySettingsLocal* Settings = UGameplaySettingsLocal::Get();
	Settings->SetPerfStatDisplayState(StatToDisplay, InitialMode);
	NotifySettingChanged(EGameplaySettingChangeReason::RestoreToInitial);
}

void UGameplaySettingValueDiscrete_PerfStat::SetDiscreteOptionByIndex(int32 Index)
{
	if (DisplayModes.IsValidIndex(Index))
	{
		const EGameplayPerformanceStatDisplayMode DisplayMode = DisplayModes[Index];

		UGameplaySettingsLocal* Settings = UGameplaySettingsLocal::Get();
		Settings->SetPerfStatDisplayState(StatToDisplay, DisplayMode);
	}

	NotifySettingChanged(EGameplaySettingChangeReason::Change);
}

int32 UGameplaySettingValueDiscrete_PerfStat::GetDiscreteOptionIndex() const
{
	const UGameplaySettingsLocal* Settings = UGameplaySettingsLocal::Get();
	return DisplayModes.Find(Settings->GetPerfStatDisplayState(StatToDisplay));
}

TArray<FText> UGameplaySettingValueDiscrete_PerfStat::GetDiscreteOptions() const
{
	return Options;
}

// ===================================================
// UGameplaySettingValueDiscrete_Language
// ===================================================

UGameplaySettingValueDiscrete_Language::UGameplaySettingValueDiscrete_Language()
{
}

void UGameplaySettingValueDiscrete_Language::OnInitialized()
{
	Super::OnInitialized();
	
	const TArray<FString> AllCultureNames = FTextLocalizationManager::Get().GetLocalizedCultureNames(ELocalizationLoadFlags::Game);
	for (const FString& CultureName : AllCultureNames)
	{
		if (FInternationalization::Get().IsCultureAllowed(CultureName))
		{
			AvailableCultureNames.Add(CultureName);
		}
	}
	
	AvailableCultureNames.Insert(TEXT(""), SettingSystemDefaultLanguageIndex);
}

void UGameplaySettingValueDiscrete_Language::StoreInitial()
{
	// Ignored.
}

void UGameplaySettingValueDiscrete_Language::OnApply()
{
	//@TODO: Implement Confirmation Dialog.
	/*if (UGameplayCommonLocalPlayerSubsystem* CommonLocalPlayerSubsystem = LocalPlayer->GetSubsystem<UGameplayCommonLocalPlayerSubsystem>())
	{
		CommonLocalPlayerSubsystem->ShowConfirmation(UGameplayConfirmationDescriptor::CreateConfirmationOk(
			LOCTEXT("WarningLanguage_Title", "Language Changed"),
			LOCTEXT("WarningLanguage_Message", "You will need to restart the game completely for all language related changes to take effect."))
		);
	}*/
}

void UGameplaySettingValueDiscrete_Language::ResetToDefault()
{
	SetDiscreteOptionByIndex(SettingSystemDefaultLanguageIndex);
}

void UGameplaySettingValueDiscrete_Language::RestoreToInitial()
{
	if (const IGameplayCommonSettingsInterface* AccessorInterface = Cast<IGameplayCommonSettingsInterface>(LocalPlayer))
	{
		if (UGameplaySettingsShared* SharedSettings = AccessorInterface->GetSharedSettings())
		{
			SharedSettings->ClearPendingCulture();
			NotifySettingChanged(EGameplaySettingChangeReason::RestoreToInitial);
		}
	}
}

void UGameplaySettingValueDiscrete_Language::SetDiscreteOptionByIndex(int32 Index)
{
	if (const IGameplayCommonSettingsInterface* AccessorInterface = Cast<IGameplayCommonSettingsInterface>(LocalPlayer))
	{
		if (UGameplaySettingsShared* SharedSettings = AccessorInterface->GetSharedSettings())
		{
			if (Index == SettingSystemDefaultLanguageIndex)
			{
				SharedSettings->ResetCultureToCurrentSettings();
			}
			else if (AvailableCultureNames.IsValidIndex(Index))
			{
				SharedSettings->SetPendingCulture(AvailableCultureNames[Index]);
			}
			
			NotifySettingChanged(EGameplaySettingChangeReason::Change);
		}
	}
}

int32 UGameplaySettingValueDiscrete_Language::GetDiscreteOptionIndex() const
{
	if (const IGameplayCommonSettingsInterface* AccessorInterface = Cast<IGameplayCommonSettingsInterface>(LocalPlayer))
	{
		if (const UGameplaySettingsShared* SharedSettings = AccessorInterface->GetSharedSettings())
		{
			if (SharedSettings->ShouldResetToDefaultCulture())
			{
				return SettingSystemDefaultLanguageIndex;
			}

			// We prefer the pending culture to the current culture as the options UI updates the pending culture before it 
			// gets applied, and we need the UI to reflect that choice
			FString PendingCulture = SharedSettings->GetPendingCulture();
			if (PendingCulture.IsEmpty())
			{
				if (SharedSettings->IsUsingDefaultCulture())
				{
					return SettingSystemDefaultLanguageIndex;
				}

				PendingCulture = FInternationalization::Get().GetCurrentCulture()->GetName();
			}

			// Try to find an exact match 
			{
				const int32 ExactMatchIndex = AvailableCultureNames.IndexOfByKey(PendingCulture);
				if (ExactMatchIndex != INDEX_NONE)
				{
					return ExactMatchIndex;
				}
			}

			// Try to find a prioritized match (eg, allowing "en-US" to show as "en" in the UI)
			const TArray<FString> PrioritizedPendingCultures = FInternationalization::Get().GetPrioritizedCultureNames(PendingCulture);
			for (int32 i = 0; i < AvailableCultureNames.Num(); ++i)
			{
				if (PrioritizedPendingCultures.Contains(AvailableCultureNames[i]))
				{
					return i;
				}
			}
		}
	}

	return 0;
}

TArray<FText> UGameplaySettingValueDiscrete_Language::GetDiscreteOptions() const
{
	TArray<FText> Options;

	for (const FString& CultureName : AvailableCultureNames)
	{
		if (CultureName == TEXT(""))
		{
			const FCulturePtr SystemDefaultCulture = FInternationalization::Get().GetDefaultCulture();
			if (ensure(SystemDefaultCulture))
			{
				const FString& DefaultCultureDisplayName = SystemDefaultCulture->GetDisplayName();
				FText LocalizedSystemDefault = FText::Format(LOCTEXT("SystemDefaultLanguage", "System Default ({0})"), FText::FromString(DefaultCultureDisplayName));

				Options.Add(MoveTemp(LocalizedSystemDefault));
			}
		}
		else
		{
			const FCulturePtr Culture = FInternationalization::Get().GetCulture(CultureName);
			if (ensureMsgf(Culture, TEXT("Unable to find Culture '%s'!"), *CultureName))
			{
				const FString CultureDisplayName = Culture->GetDisplayName();
				const FString CultureNativeName = Culture->GetNativeName();

				// Only show both names if they're different (to avoid repetition)
				FString Entry = (!CultureNativeName.Equals(CultureDisplayName, ESearchCase::CaseSensitive))
					? FString::Printf(TEXT("%s (%s)"), *CultureNativeName, *CultureDisplayName)
					: CultureNativeName;

				Options.Add(FText::FromString(Entry));
			}
		}
	}

	return Options;
}

#undef LOCTEXT_NAMESPACE
