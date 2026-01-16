// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplaySettingScreen.h"
#include "Widgets/GameplaySettingPanel.h"
#include "Framework/GameplaySettingCollection.h"
#include "Framework/GameplaySettingRegistry.h"
#include "Input/CommonUIInputTypes.h"
#include "Misc/GameplayCommonLogs.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingScreen)

#define LOCTEXT_NAMESPACE "GameSetting"

UGameplaySettingScreen::UGameplaySettingScreen()
{
	RegistryClass = nullptr;
	InputMode = EGameplayActivatableInputMode::Menu;
}

void UGameplaySettingScreen::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (ApplyAction)
	{
		ApplyActionBindingHandle = RegisterUIActionBinding(FBindUIActionArgs(ApplyAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleApplyAction)));
	}

	if (CancelAction)
	{
		CancelActionBindingHandle = RegisterUIActionBinding(FBindUIActionArgs(CancelAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleCancelAction)));
	}

	if (ReturnAction)
	{
		ReturnActionBindingHandle = RegisterUIActionBinding(FBindUIActionArgs(ReturnAction, true, FSimpleDelegate::CreateUObject(this, &ThisClass::HandleReturnAction)));
	}
}

void UGameplaySettingScreen::NativeOnActivated()
{
	Super::NativeOnActivated();

	ChangeTracker.WatchRegistry(Registry);
	OnSettingsDirtyStateChanged(HaveSettingsBeenChanged());
}

UGameplaySettingRegistry* UGameplaySettingScreen::GetOrCreateRegistry()
{
	if (Registry == nullptr)
	{
		if (RegistryClass.Get())
		{
			UGameplaySettingRegistry* NewRegistry = this->CreateRegistry();
			NewRegistry->OnSettingChangedEvent.AddUObject(this, &ThisClass::HandleSettingChanged);

			Settings_Panel->SetRegistry(NewRegistry);

			Registry = NewRegistry;
		}
	}

	return Registry;
}

void UGameplaySettingScreen::HandleApplyAction()
{
	ApplyChanges();
}

void UGameplaySettingScreen::HandleCancelAction()
{
	CancelChanges();
}

void UGameplaySettingScreen::HandleReturnAction()
{
	if (AttemptToPopNavigation())
	{
		return;
	}

	ApplyChanges();
	DeactivateWidget();
}

UWidget* UGameplaySettingScreen::NativeGetDesiredFocusTarget() const
{
	if (UWidget* Target = BP_GetDesiredFocusTarget())
	{
		return Target;
	}

	return Settings_Panel;
}

void UGameplaySettingScreen::ApplyChanges()
{
	if (ChangeTracker.HaveSettingsBeenChanged())
	{
		ChangeTracker.ApplyChanges();
		ClearDirtyState();
		Registry->SaveChanges();
	}
}

void UGameplaySettingScreen::CancelChanges()
{
	ChangeTracker.RestoreToInitial();
	ClearDirtyState();
}

void UGameplaySettingScreen::ClearDirtyState()
{
	ChangeTracker.ClearDirtyState();

	OnSettingsDirtyStateChanged(false);
}

bool UGameplaySettingScreen::AttemptToPopNavigation()
{
	if (Settings_Panel->CanPopNavigationStack())
	{
		Settings_Panel->PopNavigationStack();
		return true;
	}

	return false;
}

UGameplaySettingCollection* UGameplaySettingScreen::GetSettingCollection(FName SettingDevName, bool& HasAnySettings)
{
	HasAnySettings = false;
	
	if (GetRegistry())
	{
		if (UGameplaySettingCollection* Collection = GetRegistry()->FindSettingByDevNameChecked<UGameplaySettingCollection>(SettingDevName))
		{
			TArray<UGameplaySetting*> InOutSettings;
		
			const FGameplaySettingFilterState FilterState;
			Collection->GetSettingsForFilter(FilterState, InOutSettings);

			HasAnySettings = InOutSettings.Num() > 0;
		
			return Collection;
		}
	}
	
	COMMON_SETTINGS_LOG(Error, TEXT("Registry Class is not valid, please check your game setting widget."));
	return nullptr;
}

UGameplaySettingRegistry* UGameplaySettingScreen::CreateRegistry()
{
	UGameplaySettingRegistry* NewRegistry = NewObject<UGameplaySettingRegistry>(this, RegistryClass);
	
	if (ULocalPlayer* LocalPlayer = GetOwningLocalPlayer())
	{
		NewRegistry->Initialize(LocalPlayer);
	}

	return NewRegistry;
}

void UGameplaySettingScreen::NavigateToSetting(FName SettingDevName)
{
	NavigateToSettings({SettingDevName});
}

void UGameplaySettingScreen::NavigateToSettings(const TArray<FName>& SettingDevNames)
{
	FGameplaySettingFilterState FilterState;

	for (const FName SettingDevName : SettingDevNames)
	{
		if (UGameplaySetting* Setting = GetRegistry()->FindSettingByDevNameChecked<UGameplaySetting>(SettingDevName))
		{
			FilterState.AddSettingToRootList(Setting);
		}
	}
	
	Settings_Panel->SetFilterState(FilterState);
}

void UGameplaySettingScreen::OnSettingsDirtyStateChanged_Implementation(bool bSettingsDirty)
{
	if (bSettingsDirty)
	{
		if (!GetActionBindings().Contains(ApplyActionBindingHandle))
		{
			AddActionBinding(ApplyActionBindingHandle);
		}
		if (!GetActionBindings().Contains(CancelActionBindingHandle))
		{
			AddActionBinding(CancelActionBindingHandle);
		}
	}
	else
	{
		RemoveActionBinding(ApplyActionBindingHandle);
		RemoveActionBinding(CancelActionBindingHandle);
	}
}

void UGameplaySettingScreen::HandleSettingChanged(UGameplaySetting* Setting, EGameplaySettingChangeReason Reason)
{
	OnSettingsDirtyStateChanged(true);
}

#undef LOCTEXT_NAMESPACE
