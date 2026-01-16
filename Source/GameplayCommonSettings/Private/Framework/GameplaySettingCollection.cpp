// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingCollection.h"
#include "Framework/GameplaySettingFilterState.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingCollection)

/**
 * UGameplaySettingCollection
 */
UGameplaySettingCollection::UGameplaySettingCollection(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameplaySettingCollection::AddSetting(UGameplaySetting* InSetting)
{
#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(InSetting->GetSettingParent() == nullptr, TEXT("This setting already has a parent!"));
	ensureAlwaysMsgf(!Settings.Contains(InSetting), TEXT("This collection already includes this setting!"));
#endif

	Settings.Add(InSetting);
	InSetting->SetSettingParent(this);

	if (LocalPlayer)
	{
		InSetting->Initialize(LocalPlayer);
	}
}

TArray<UGameplaySettingCollection*> UGameplaySettingCollection::GetChildCollection() const
{
	TArray<UGameplaySettingCollection*> CollectionSettings;

	for (UGameplaySetting* ChildSetting : Settings)
	{
		if (UGameplaySettingCollection* ChildCollection = Cast<UGameplaySettingCollection>(ChildSetting))
		{
			CollectionSettings.Add(ChildCollection);
		}
	}

	return CollectionSettings;
}

void UGameplaySettingCollection::GetSettingsForFilter(const FGameplaySettingFilterState& InFilterState, TArray<UGameplaySetting*>& InOutSettings) const
{
	for (UGameplaySetting* ChildSetting : Settings)
	{
		// If the child setting is a collection, only add it to the set if it has any visible children.
		if (Cast<UGameplaySettingCollectionPage>(ChildSetting))
		{
			if (InFilterState.DoesSettingPassFilter(*ChildSetting))
			{
				InOutSettings.Add(ChildSetting);
			}
		}
		else if (const UGameplaySettingCollection* ChildCollection = Cast<UGameplaySettingCollection>(ChildSetting))
		{
			TArray<UGameplaySetting*> ChildSettings;
			ChildCollection->GetSettingsForFilter(InFilterState, ChildSettings);

			if (ChildSettings.Num() > 0)
			{
				// Don't add the root setting to the returned items, it's the container of N-possible 
				// other settings and containers we're actually displaying right now.
				if (!InFilterState.IsSettingInRootList(ChildSetting))
				{
					InOutSettings.Add(ChildSetting);
				}

				InOutSettings.Append(ChildSettings);
			}
		}
		else
		{
			if (InFilterState.DoesSettingPassFilter(*ChildSetting))
			{
				InOutSettings.Add(ChildSetting);
			}
		}
	}
}

/**
 * UGameplaySettingCollectionPage
 */
UGameplaySettingCollectionPage::UGameplaySettingCollectionPage(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	NavigationText = FText::GetEmpty();
}

void UGameplaySettingCollectionPage::OnInitialized()
{
	Super::OnInitialized();

#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!NavigationText.IsEmpty(), TEXT("You must provide a NavigationText for this setting."));
	ensureAlwaysMsgf(!DescriptionRichText.IsEmpty(), TEXT("You must provide a description for new page collections."));
#endif
}

void UGameplaySettingCollectionPage::GetSettingsForFilter(const FGameplaySettingFilterState& InFilterState, TArray<UGameplaySetting*>& InOutSettings) const
{
	// If we're including nested pages, call the super and dump them all, otherwise, we pretend we have none for the filtering.
	// because our settings are displayed on another page.
	if (InFilterState.bIncludeNestedPages || InFilterState.IsSettingInRootList(this))
	{
		Super::GetSettingsForFilter(InFilterState, InOutSettings);
	}
}

void UGameplaySettingCollectionPage::ExecuteNavigation()
{
	OnExecuteNavigationEvent.Broadcast(this);
}
