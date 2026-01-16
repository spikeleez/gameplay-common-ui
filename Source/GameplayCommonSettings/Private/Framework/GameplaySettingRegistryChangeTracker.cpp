// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingRegistryChangeTracker.h"
#include "Framework/GameplaySetting.h"
#include "Framework/GameplaySettingRegistry.h"
#include "Framework/GameplaySettingValue.h"

#define LOCTEXT_NAMESPACE "GameplayGameSettingRegistryChangeTracker"

FGameplaySettingRegistryChangeTracker::FGameplaySettingRegistryChangeTracker()
{
	bSettingsChanged = false;
	bRestoringSettings = false;
}

FGameplaySettingRegistryChangeTracker::~FGameplaySettingRegistryChangeTracker()
{
	if (UGameplaySettingRegistry* StrongRegistry = Registry.Get())
	{
		StrongRegistry->OnSettingChangedEvent.RemoveAll(this);
	}
}

void FGameplaySettingRegistryChangeTracker::WatchRegistry(UGameplaySettingRegistry* InRegistry)
{
	ClearDirtyState();
	StopWatchRegistry();

	if (Registry.Get() != InRegistry)
	{
		Registry = InRegistry;
		InRegistry->OnSettingChangedEvent.AddRaw(this, &FGameplaySettingRegistryChangeTracker::HandleSettingChanged);
	}
}

void FGameplaySettingRegistryChangeTracker::StopWatchRegistry()
{
	if (UGameplaySettingRegistry* StrongRegistry = Registry.Get())
	{
		StrongRegistry->OnSettingChangedEvent.RemoveAll(this);
		Registry.Reset();
	}
}

void FGameplaySettingRegistryChangeTracker::ApplyChanges()
{
	for (TPair<FObjectKey, TWeakObjectPtr<UGameplaySetting>> Entry : DirtySettings)
	{
		if (UGameplaySettingValue* SettingValue = Cast<UGameplaySettingValue>(Entry.Value.Get()))
		{
			SettingValue->Apply();
			SettingValue->StoreInitial();
		}
	}

	ClearDirtyState();
}

void FGameplaySettingRegistryChangeTracker::RestoreToInitial()
{
	ensure(!bRestoringSettings);
	if (bRestoringSettings)
	{
		return;
	}

	{
		TGuardValue<bool> LocalGuard(bRestoringSettings, true);
		for (TPair<FObjectKey, TWeakObjectPtr<UGameplaySetting>> Entry : DirtySettings)
		{
			if (UGameplaySettingValue* SettingValue = Cast<UGameplaySettingValue>(Entry.Value.Get()))
			{
				SettingValue->RestoreToInitial();
			}
		}
	}

	ClearDirtyState();
}

void FGameplaySettingRegistryChangeTracker::ClearDirtyState()
{
	ensure(!bRestoringSettings);
	if (bRestoringSettings)
	{
		return;
	}

	bSettingsChanged = false;
	DirtySettings.Reset();
}

void FGameplaySettingRegistryChangeTracker::HandleSettingChanged(UGameplaySetting* Setting, EGameplaySettingChangeReason Reason)
{
	if (bRestoringSettings)
	{
		return;
	}

	bSettingsChanged = true;
	DirtySettings.Add(FObjectKey(Setting), Setting);
}

#undef LOCTEXT_NAMESPACE
