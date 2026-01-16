// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingVisualData.h"
#include "Widgets/GameplaySettingListEntry.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingVisualData)

UGameplaySettingVisualData::UGameplaySettingVisualData(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

TSubclassOf<UGameplaySettingListEntryBase> UGameplaySettingVisualData::GetEntryForSetting(UGameplaySetting* InSetting)
{
	if (InSetting == nullptr)
	{
		return TSubclassOf<UGameplaySettingListEntryBase>();
	}

	// Check if there's a custom logic for finding this setting's visual element.
	TSubclassOf<UGameplaySettingListEntryBase> CustomEntry = GetCustomEntryForSetting(InSetting);
	if (CustomEntry)
	{
		return CustomEntry;
	}

	// Check if there's a specific entry widget for a setting by name.  Hopefully this is super rare.
	{
		TSubclassOf<UGameplaySettingListEntryBase> EntryWidgetClassPtr = EntryWidgetForName.FindRef(InSetting->GetDevName());
		if (EntryWidgetClassPtr)
		{
			return EntryWidgetClassPtr;
		}
	}

	// Finally, check to see if there's an entry for this setting following the classes we have entries for.
	// We use the super chain of the setting classes to find the most applicable entry widget for this class
	// of setting.
	for (UClass* Class = InSetting->GetClass(); Class; Class = Class->GetSuperClass())
	{
		if (TSubclassOf<UGameplaySetting> SettingClass = TSubclassOf<UGameplaySetting>(Class))
		{
			TSubclassOf<UGameplaySettingListEntryBase> EntryWidgetClassPtr = EntryWidgetForClass.FindRef(SettingClass);
			if (EntryWidgetClassPtr)
			{
				return EntryWidgetClassPtr;
			}
		}
	}

	return TSubclassOf<UGameplaySettingListEntryBase>();
}

TArray<TSoftClassPtr<UGameplaySettingDetailExtension>> UGameplaySettingVisualData::GatherDetailExtensions(UGameplaySetting* InSetting)
{
	TArray<TSoftClassPtr<UGameplaySettingDetailExtension>> Extensions;

	// Find extensions by setting name.
	const FGameplaySettingNameExtensions* ExtensionsWithName = ExtensionsForName.Find(InSetting->GetDevName());
	if (ExtensionsWithName)
	{
		Extensions.Append(ExtensionsWithName->Extensions);
		if (!ExtensionsWithName->bIncludeClassDefaultExtensions)
		{
			return Extensions;
		}
	}

	// Find extensions for it using the super chain of the setting so that we get any
	// class-based extensions for this setting.
	for (UClass* Class = InSetting->GetClass(); Class; Class = Class->GetSuperClass())
	{
		if (TSubclassOf<UGameplaySetting> SettingClass = TSubclassOf<UGameplaySetting>(Class))
		{
			const FGameplaySettingClassExtensions* ExtensionForClass = ExtensionsForClasses.Find(SettingClass);
			if (ExtensionForClass)
			{
				Extensions.Append(ExtensionForClass->Extensions);
			}
		}
	}

	return Extensions;
}

TSubclassOf<UGameplaySettingListEntryBase> UGameplaySettingVisualData::GetCustomEntryForSetting(UGameplaySetting* InSetting)
{
	return TSubclassOf<UGameplaySettingListEntryBase>();
}
