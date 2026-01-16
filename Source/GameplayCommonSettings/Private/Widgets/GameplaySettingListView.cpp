// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplaySettingListView.h"
#include "Framework/GameplaySetting.h"
#include "Framework/GameplaySettingCollection.h"
#include "Framework/GameplaySettingVisualData.h"
#include "Widgets/GameplaySettingListEntry.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingListView)

UGameplaySettingListView::UGameplaySettingListView(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR
void UGameplaySettingListView::ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	if (!VisualData)
	{
		CompileLog.Error(FText::Format(FText::FromString("{0} has no VisualData defined."), FText::FromString(GetName())));
	}
}
#endif

UUserWidget& UGameplaySettingListView::OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable)
{
	UGameplaySetting* SettingItem = Cast<UGameplaySetting>(Item);
	if (!SettingItem)
	{
		return Super::OnGenerateEntryWidgetInternal(Item, DesiredEntryClass, OwnerTable);
	}

	TSubclassOf<UGameplaySettingListEntryBase> SettingEntryClass = TSubclassOf<UGameplaySettingListEntryBase>(DesiredEntryClass);
	if (VisualData)
	{
		if (const TSubclassOf<UGameplaySettingListEntryBase> EntryClassSetting = VisualData->GetEntryForSetting(SettingItem))
		{
			SettingEntryClass = EntryClassSetting;
		}
	}

	UGameplaySettingListEntryBase& EntryWidget = GenerateTypedEntry<UGameplaySettingListEntryBase>(SettingEntryClass, OwnerTable);

	if (!IsDesignTime())
	{
		if (const FText* Override = NameOverrides.Find(SettingItem->GetDevName()))
		{
			EntryWidget.SetDisplayNameOverride(*Override);
		}

		EntryWidget.SetSetting(SettingItem);
	}

	return EntryWidget;
}

bool UGameplaySettingListView::OnIsSelectableOrNavigableInternal(UObject* FirstSelectedItem)
{
	if (const UGameplaySettingCollection* CollectionItem = Cast<UGameplaySettingCollection>(FirstSelectedItem))
	{
		return CollectionItem->IsSelectable();
	}

	return true;
}

void UGameplaySettingListView::AddNameOverride(const FName& DevName, const FText& OverrideName)
{
	NameOverrides.Add(DevName, OverrideName);
}
