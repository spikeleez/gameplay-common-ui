// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplaySettingPanel.h"
#include "Widgets/GameplaySettingDetailView.h"
#include "Widgets/GameplaySettingListView.h"
#include "CommonInputSubsystem.h"
#include "Framework/GameplaySettingRegistry.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingPanel)

UGameplaySettingPanel::UGameplaySettingPanel()
{
	SetIsFocusable(true);
}

void UGameplaySettingPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	ListView_Settings->OnItemIsHoveredChanged().AddUObject(this, &ThisClass::HandleSettingItemHoveredChanged);
	ListView_Settings->OnItemSelectionChanged().AddUObject(this, &ThisClass::HandleSettingItemSelectionChanged);
}

void UGameplaySettingPanel::NativeConstruct()
{
	Super::NativeConstruct();

	UnregisterRegistryEvents();
	RegisterRegistryEvents();
}

void UGameplaySettingPanel::NativeDestruct()
{
	Super::NativeDestruct();

	UnregisterRegistryEvents();
}

FReply UGameplaySettingPanel::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	const UCommonInputSubsystem* InputSubsystem = GetInputSubsystem();
	if (InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
	{
		if (TSharedPtr<SWidget> PrimarySlateWidget = ListView_Settings->GetCachedWidget())
		{
			ListView_Settings->NavigateToIndex(0);
			ListView_Settings->SetSelectedIndex(0);

			return FReply::Handled();
		}
	}

	return FReply::Unhandled();
}

void UGameplaySettingPanel::SetRegistry(UGameplaySettingRegistry* InRegistry)
{
	if (Registry != InRegistry)
	{
		UnregisterRegistryEvents();

		if (RefreshHandle.IsValid())
		{
			FTSTicker::GetCoreTicker().RemoveTicker(RefreshHandle);
		}

		Registry = InRegistry;

		RegisterRegistryEvents();
		RefreshSettingsList();
	}
}

void UGameplaySettingPanel::RegisterRegistryEvents()
{
	if (Registry)
	{
		Registry->OnSettingEditConditionChangedEvent.AddUObject(this, &ThisClass::HandleSettingEditConditionsChanged);
		Registry->OnSettingNamedActionEvent.AddUObject(this, &ThisClass::HandleSettingNamedAction);
		Registry->OnExecuteNavigationEvent.AddUObject(this, &ThisClass::HandleSettingNavigation);
	}
}

void UGameplaySettingPanel::UnregisterRegistryEvents()
{
	if (Registry)
	{
		Registry->OnSettingEditConditionChangedEvent.RemoveAll(this);
		Registry->OnSettingNamedActionEvent.RemoveAll(this);
		Registry->OnExecuteNavigationEvent.RemoveAll(this);
	}
}

void UGameplaySettingPanel::SetFilterState(const FGameplaySettingFilterState& InFilterState, bool bClearNavigationStack)
{
	FilterState = InFilterState;

	if (bClearNavigationStack)
	{
		FilterNavigationStack.Reset();
	}

	RefreshSettingsList();
}

bool UGameplaySettingPanel::CanPopNavigationStack() const
{
	return FilterNavigationStack.Num() > 0;
}

void UGameplaySettingPanel::PopNavigationStack()
{
	if (FilterNavigationStack.Num() > 0)
	{
		FilterState = FilterNavigationStack.Pop();
		RefreshSettingsList();
	}
}

void UGameplaySettingPanel::HandleSettingNavigation(UGameplaySetting* InSetting)
{
	if (VisibleSettings.Contains(InSetting))
	{
		FilterNavigationStack.Push(FilterState);

		FGameplaySettingFilterState NewPageFilterState;
		NewPageFilterState.AddSettingToRootList(InSetting);
		SetFilterState(NewPageFilterState, false);
	}
}

TArray<UGameplaySetting*> UGameplaySettingPanel::GetSettingsWeCanResetToDefault() const
{
	TArray<UGameplaySetting*> AvailableSettings;

	if (ensure(Registry->IsFinishedInitializing()))
	{
		// We want to get all available settings on this "screen" so we include the same allowlist, but ignore.
		FGameplaySettingFilterState AllAvailableFilter = FilterState;
		AllAvailableFilter.bIncludeDisabled = true;
		AllAvailableFilter.bIncludeHidden = true;
		AllAvailableFilter.bIncludeResetable = false;
		AllAvailableFilter.bIncludeNestedPages = false;

		Registry->GetSettingsForFilter(AllAvailableFilter, AvailableSettings);
	}

	return AvailableSettings;
}

void UGameplaySettingPanel::RefreshSettingsList()
{
	if (RefreshHandle.IsValid())
	{
		return;
	}
	
	RefreshHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this](float DeltaTime)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_UGameplayGameSettingPanel_RefreshSettingsList);
		
		if (Registry->IsFinishedInitializing())
		{
			VisibleSettings.Reset();
			Registry->GetSettingsForFilter(FilterState, MutableView(VisibleSettings));
			
			ListView_Settings->SetListItems(VisibleSettings);
			
			RefreshHandle.Reset();

			int32 IndexToSelect = 0;
			if (DesiredSelectionPostRefresh != NAME_None)
			{
				for (int32 SettingIdx = 0; SettingIdx < VisibleSettings.Num(); ++SettingIdx)
				{
					UGameplaySetting* Setting = VisibleSettings[SettingIdx];
					if (Setting->GetDevName() == DesiredSelectionPostRefresh)
					{
						IndexToSelect = SettingIdx;
						break;
					}
				}

				DesiredSelectionPostRefresh = NAME_None;
			}

			// If the list directly has the focus, instead of a child widget, then it's likely the panel and items
			// were not yet available when we received focus, so lets go ahead and focus the first item now.
			//if (HasUserFocus(GetOwningPlayer()))
			if (bAdjustListViewPostRefresh)
			{
				ListView_Settings->NavigateToIndex(IndexToSelect);
				ListView_Settings->SetSelectedIndex(IndexToSelect);
			}

			bAdjustListViewPostRefresh = true;

			// finally, refresh the editable state, but only once.
			for (int32 SettingIdx = 0; SettingIdx < VisibleSettings.Num(); ++SettingIdx)
			{
				if (UGameplaySetting* Setting = VisibleSettings[SettingIdx])
				{
					Setting->RefreshEditableState(false);
				}
			}

			return false;
		}

		return true;
	}));
}

void UGameplaySettingPanel::HandleSettingItemHoveredChanged(UObject* Item, bool bHovered)
{
	UGameplaySetting* Setting = bHovered ? Cast<UGameplaySetting>(Item) : ToRawPtr(LastHoveredOrSelectedSetting);
	if (bHovered && Setting)
	{
		LastHoveredOrSelectedSetting = Setting;
	}

	FillSettingDetails(Setting);
}

void UGameplaySettingPanel::HandleSettingItemSelectionChanged(UObject* Item)
{
	UGameplaySetting* Setting = Cast<UGameplaySetting>(Item);
	if (Setting)
	{
		LastHoveredOrSelectedSetting = Setting;
	}

	FillSettingDetails(Cast<UGameplaySetting>(Item));
}

void UGameplaySettingPanel::FillSettingDetails(UGameplaySetting* InSetting)
{
	if (Details_Settings)
	{
		Details_Settings->FillSettingDetails(InSetting);
	}

	OnFocusedSettingChanged.Broadcast(InSetting);
}

void UGameplaySettingPanel::HandleSettingNamedAction(UGameplaySetting* InSetting, FGameplayTag GameSettingsActionTag)
{
	OnExecuteNamedActionBlueprint.Broadcast(InSetting, GameSettingsActionTag);
}

void UGameplaySettingPanel::HandleSettingEditConditionsChanged(UGameplaySetting* InSetting)
{
	const bool bWasSettingVisible = VisibleSettings.Contains(InSetting);
	const bool bIsSettingVisible = InSetting->GetEditState().IsVisible();

	if (bIsSettingVisible != bWasSettingVisible)
	{
		bAdjustListViewPostRefresh = InSetting->GetAdjustListViewPostRefresh();
		RefreshSettingsList();
	}
}

void UGameplaySettingPanel::SelectSetting(const FName& SettingDevName)
{
	DesiredSelectionPostRefresh = SettingDevName;
	RefreshSettingsList();
}

UGameplaySetting* UGameplaySettingPanel::GetSelectedSetting() const
{
	return Cast<UGameplaySetting>(ListView_Settings->GetSelectedItem());
}
