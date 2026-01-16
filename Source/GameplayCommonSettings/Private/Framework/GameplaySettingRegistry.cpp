// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingRegistry.h"
#include "Framework/GameplaySettingAction.h"
#include "Framework/GameplaySettingCollection.h"
#include "Framework/GameplaySettingFilterState.h"
#include "Framework/GameplaySettingsLocal.h"
#include "Framework/GameplaySettingsShared.h"
#include "Misc/GameplayCommonLogs.h"
#include "Misc/GameplayCommonSettingsLibrary.h"

#define LOCTEXT_NAMESPACE "GameplaySettingRegistry"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingRegistry)

UGameplaySettingRegistry::UGameplaySettingRegistry(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameplaySettingRegistry::Initialize(ULocalPlayer* InLocalPlayer)
{
	OwningLocalPlayer = InLocalPlayer;
	
	if (!OwningLocalPlayer)
	{
		COMMON_SETTINGS_LOG(Fatal, TEXT("Local Player is null! Registry initialization failed."));
		return;
	}
	
	COMMON_SETTINGS_LOG(Log, TEXT("Initializing registry [%s] for player [%s]"), *GetNameSafe(this), *InLocalPlayer->GetName());
	
	OnInitialize(InLocalPlayer);
}

void UGameplaySettingRegistry::Regenerate()
{
	COMMON_SETTINGS_LOG(Log, TEXT("Regenerating settings tree..."));
	
	if (TopLevelSettings.Num() > 0)
	{
		COMMON_SETTINGS_LOG(Verbose, TEXT("Clearing %d existing top-level settings."), TopLevelSettings.Num());
		TopLevelSettings.Reset();
	}
	
	if (RegisteredSettings.Num() > 0)
	{
		COMMON_SETTINGS_LOG(Verbose, TEXT("Clearing %d existing registered settings."), RegisteredSettings.Num());
		
		for (UGameplaySetting* Setting : RegisteredSettings)
		{
			Setting->MarkAsGarbage();
		}
		
		RegisteredSettings.Reset();
	}
	
	OnInitialize(OwningLocalPlayer);
}

bool UGameplaySettingRegistry::IsFinishedInitializing() const
{
	bool bReady = true;
	for (const UGameplaySetting* Setting : RegisteredSettings)
	{
		if (!Setting->IsReady())
		{
			bReady = false;
			break;
		}
	}
	
	if (bReady)
	{
		if (!UGameplayCommonSettingsLibrary::GetSettingsShared(OwningLocalPlayer))
		{
			return false;
		}
	}

	return bReady;
}

void UGameplaySettingRegistry::SaveChanges()
{
	if (UGameplaySettingsLocal* LocalSettings = UGameplayCommonSettingsLibrary::GetLocalSettings(OwningLocalPlayer))
	{
		// Game user settings need to be applied to handle things like resolution, this save indirectly.
		LocalSettings->ApplySettings(false);
		
		if (UGameplaySettingsShared* SharedSettings = UGameplayCommonSettingsLibrary::GetSettingsShared(OwningLocalPlayer))
		{
			SharedSettings->ApplySettings();
			SharedSettings->SaveSettings();
		}
	}
}

void UGameplaySettingRegistry::GetSettingsForFilter(const FGameplaySettingFilterState& InFilterState, TArray<UGameplaySetting*>& InOutSettings)
{
	TArray<UGameplaySetting*> RootSettings;
	if (InFilterState.GetSettingRootList().Num() > 0)
	{
		RootSettings.Append(InFilterState.GetSettingRootList());
	}
	else
	{
		RootSettings.Append(TopLevelSettings);
	}

	for (UGameplaySetting* TopLevelSetting : RootSettings)
	{
		if (const UGameplaySettingCollection* TopLevelCollection = Cast<UGameplaySettingCollection>(TopLevelSetting))
		{
			TopLevelCollection->GetSettingsForFilter(InFilterState, InOutSettings);
		}
		else
		{
			if (InFilterState.DoesSettingPassFilter(*TopLevelSetting))
			{
				InOutSettings.Add(TopLevelSetting);
			}
		}
	}
}

UGameplaySetting* UGameplaySettingRegistry::FindSettingByDevName(const FName& InSettingDevName)
{
	for (UGameplaySetting* Setting : RegisteredSettings)
	{
		if (Setting->GetDevName() == InSettingDevName)
		{
			return Setting;
		}
	}

	return nullptr;
}

void UGameplaySettingRegistry::RegisterSetting(UGameplaySetting* InSetting)
{
	if (InSetting)
	{
		TopLevelSettings.Add(InSetting);
		InSetting->SetRegistry(this);
		RegisterInnerSetting(InSetting);
	}
}

void UGameplaySettingRegistry::RegisterInnerSetting(UGameplaySetting* InSetting)
{
	InSetting->OnSettingChangedEvent.AddUObject(this, &ThisClass::HandleSettingChanged);
	InSetting->OnSettingAppliedEvent.AddUObject(this, &ThisClass::HandleSettingApplied);
	InSetting->OnSettingEditConditionChangedEvent.AddUObject(this, &ThisClass::HandleSettingEditConditionsChanged);

	if (UGameplaySettingAction* ActionSetting = Cast<UGameplaySettingAction>(InSetting))
	{
		ActionSetting->OnExecuteNamedActionEvent.AddUObject(this, &ThisClass::HandleSettingNamedAction);
	}
	else if (UGameplaySettingCollectionPage* NewPageCollection = Cast<UGameplaySettingCollectionPage>(InSetting))
	{
		NewPageCollection->OnExecuteNavigationEvent.AddUObject(this, &ThisClass::HandleSettingNavigation);
	}

#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!RegisteredSettings.Contains(InSetting), TEXT("This setting has already been registered!"));
	ensureAlwaysMsgf(nullptr == RegisteredSettings.FindByPredicate([&InSetting](const UGameplaySetting* ExistingSetting)
	{
		return InSetting->GetDevName() == ExistingSetting->GetDevName();
	}), TEXT("A setting with this DevName has already been registered!  DevNames must be unique within a registry."));
#endif

	RegisteredSettings.Add(InSetting);

	for (UGameplaySetting* ChildSetting : InSetting->GetChildSettings())
	{
		RegisterInnerSetting(ChildSetting);
	}
}

void UGameplaySettingRegistry::HandleSettingChanged(UGameplaySetting* Setting, EGameplaySettingChangeReason Reason)
{
	OnSettingChangedEvent.Broadcast(Setting, Reason);
}

void UGameplaySettingRegistry::HandleSettingApplied(UGameplaySetting* Setting)
{
	OnSettingApplied(Setting);
}

void UGameplaySettingRegistry::HandleSettingEditConditionsChanged(UGameplaySetting* Setting)
{
	OnSettingEditConditionChangedEvent.Broadcast(Setting);
}

void UGameplaySettingRegistry::HandleSettingNamedAction(UGameplaySetting* Setting, FGameplayTag ActionTag)
{
	OnSettingNamedActionEvent.Broadcast(Setting, ActionTag);
}

void UGameplaySettingRegistry::HandleSettingNavigation(UGameplaySetting* Setting)
{
	OnExecuteNavigationEvent.Broadcast(Setting);
}

#undef LOCTEXT_NAMESPACE
