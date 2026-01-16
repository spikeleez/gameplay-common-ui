// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ListView.h"
#include "GameplaySettingListView.generated.h"

class UGameplaySettingVisualData;

/**
 * @brief A specialized ListView for displaying and interacting with game settings
 *
 * This widget automates the creation of setting entries. It uses UGameplaySettingVisualData 
 * to determine which entry class (slider, rotator, etc.) to instantiate based 
 * on the type of setting object being displayed.
 * 
 * Every entry widget managed by this list must inherit from UGameplaySettingListEntryBase.
 */
UCLASS(meta=(EntryClass = GameplaySettingListEntryBase))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingListView : public UListView
{
	GENERATED_BODY()

public:
	UGameplaySettingListView(const FObjectInitializer& ObjectInitializer);
	
	/**
	 * @brief Registers a display name override for a specific setting
	 * @param DevName The developer name (internal ID) of the setting
	 * @param OverrideName The customer-facing text to display instead
	 */
	void AddNameOverride(const FName& DevName, const FText& OverrideName);

#if WITH_EDITOR
	virtual void ValidateCompiledDefaults(class IWidgetCompilerLog& CompileLog) const override;
#endif

protected:
	/** Data Asset mapping setting classes to their respective entry widget classes */
	UPROPERTY(EditAnywhere, Category="Visual")
	TObjectPtr<UGameplaySettingVisualData> VisualData;
	
protected:
	// ~Begin UListView interface
	virtual UUserWidget& OnGenerateEntryWidgetInternal(UObject* Item, TSubclassOf<UUserWidget> DesiredEntryClass, const TSharedRef<STableViewBase>& OwnerTable) override;
	virtual bool OnIsSelectableOrNavigableInternal(UObject* FirstSelectedItem) override;
	// ~End of UListView interface

private:
	/** Map of internal setting names to their localized display overrides */
	TMap<FName, FText> NameOverrides;
};