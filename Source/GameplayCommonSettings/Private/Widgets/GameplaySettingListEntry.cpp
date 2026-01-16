// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplaySettingListEntry.h"
#include "Widgets/GameplaySettingRotator.h"
#include "AnalogSlider.h"
#include "CommonInputSubsystem.h"
#include "CommonTextBlock.h"
#include "Framework/GameplaySettingAction.h"
#include "Framework/GameplaySettingCollection.h"
#include "Framework/GameplaySettingValueDiscrete.h"
#include "Framework/GameplaySettingValueKeyboard.h"
#include "Framework/GameplaySettingValueScalar.h"
#include "Misc/GameplayCommonTags.h"
#include "Misc/GameplayCommonUILibrary.h"
#include "Widgets/GameplayButtonBase.h"
#include "Widgets/GameplaySettingKeyAlreadyBound.h"
#include "Widgets/GameplaySettingPressAnyKey.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingListEntry)

#define LOCTEXT_NAMESPACE "GameplaySettingListEntry"

//=========================================================
// UGameplaySettingListEntryBase
//=========================================================
void UGameplaySettingListEntryBase::SetSetting(UGameplaySetting* InSetting)
{
	Setting = InSetting;
	Setting->OnSettingEditConditionChangedEvent.AddUObject(this, &UGameplaySettingListEntryBase::HandleEditConditionChanged);
	Setting->OnSettingChangedEvent.AddUObject(this, &UGameplaySettingListEntryBase::HandleSettingChanged);

	HandleEditConditionChanged(Setting);
}

void UGameplaySettingListEntryBase::SetDisplayNameOverride(const FText& OverrideName)
{
	DisplayNameOverride = OverrideName;
}

void UGameplaySettingListEntryBase::NativeOnEntryReleased()
{
	StopAllAnimations();

	if (Background)
	{
		Background->StopAllAnimations();
	}

	if (ensure(Setting))
	{
		Setting->OnSettingEditConditionChangedEvent.RemoveAll(this);
		Setting->OnSettingChangedEvent.RemoveAll(this);
	}

	Setting = nullptr;
}

void UGameplaySettingListEntryBase::HandleSettingChanged(UGameplaySetting* InSetting, EGameplaySettingChangeReason Reason)
{
	if (!bSuspendChangeUpdates)
	{
		OnSettingChanged();
	}
}

void UGameplaySettingListEntryBase::OnSettingChanged()
{
	// No-op
}

void UGameplaySettingListEntryBase::HandleEditConditionChanged(UGameplaySetting* InSetting)
{
	const FGameplaySettingEditableState EditableState = Setting->GetEditState();
	RefreshEditableState(EditableState);
}

void UGameplaySettingListEntryBase::RefreshEditableState(const FGameplaySettingEditableState& InEditableState)
{
	// No-op
}

FReply UGameplaySettingListEntryBase::NativeOnFocusReceived(const FGeometry& InGeometry, const FFocusEvent& InFocusEvent)
{
	const UCommonInputSubsystem* InputSubsystem = GetInputSubsystem();
	if (InputSubsystem && InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad)
	{
		if (const UWidget* PrimaryFocus = GetPrimaryGamepadFocusWidget())
		{
			const TSharedPtr<SWidget> WidgetToFocus = PrimaryFocus->GetCachedWidget();
			if (WidgetToFocus.IsValid())
			{
				return FReply::Handled().SetUserFocus(WidgetToFocus.ToSharedRef(), InFocusEvent.GetCause());
			}
		}
	}

	return FReply::Unhandled();
}

//=========================================================
// UGameplaySettingListEntry_Setting
//=========================================================
void UGameplaySettingListEntry_Setting::SetSetting(UGameplaySetting* InSetting)
{
	Super::SetSetting(InSetting);

	Text_SettingName->SetText(DisplayNameOverride.IsEmpty() ? Setting->GetDisplayName() : DisplayNameOverride);
	Text_SettingName->SetVisibility(InSetting->GetDisplayNameVisibility());
}

void UGameplaySettingListEntry_Setting::RefreshEditableState(const FGameplaySettingEditableState& InEditableState)
{
	const bool bLocalIsEnabled = InEditableState.IsEnabled();
	Text_SettingName->SetIsEnabled(bLocalIsEnabled);
}

//=========================================================
// UGameplaySettingListEntry_SettingDiscrete
//=========================================================
void UGameplaySettingListEntry_SettingDiscrete::SetSetting(UGameplaySetting* InSetting)
{
	DiscreteSetting = Cast<UGameplaySettingValueDiscrete>(InSetting);
	
	Super::SetSetting(InSetting);

	Refresh();
}

void UGameplaySettingListEntry_SettingDiscrete::OnSettingChanged()
{
	Refresh();
}

void UGameplaySettingListEntry_SettingDiscrete::Refresh()
{
	if (ensure(DiscreteSetting))
	{
		const TArray<FText> Options = DiscreteSetting->GetDiscreteOptions();
		ensure(Options.Num() > 0);

		Rotator_SettingValue->PopulateTextLabels(Options);
		Rotator_SettingValue->SetSelectedItem(DiscreteSetting->GetDiscreteOptionIndex());
		Rotator_SettingValue->SetDefaultOption(DiscreteSetting->GetDiscreteOptionDefaultIndex());
	}
}

void UGameplaySettingListEntry_SettingDiscrete::RefreshEditableState(const FGameplaySettingEditableState& InEditableState)
{
	Super::RefreshEditableState(InEditableState);

	const bool bLocalIsEnabled = InEditableState.IsEnabled();
	Button_Decrease->SetIsEnabled(bLocalIsEnabled);
	Rotator_SettingValue->SetIsEnabled(bLocalIsEnabled);
	Button_Increase->SetIsEnabled(bLocalIsEnabled);
	Panel_Value->SetIsEnabled(bLocalIsEnabled);
}

void UGameplaySettingListEntry_SettingDiscrete::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Rotator_SettingValue->OnRotatedEvent.AddUObject(this, &UGameplaySettingListEntry_SettingDiscrete::HandleRotatorChangedValue);
	Button_Decrease->OnClicked().AddUObject(this, &UGameplaySettingListEntry_SettingDiscrete::HandleOptionDecrease);
	Button_Increase->OnClicked().AddUObject(this, &UGameplaySettingListEntry_SettingDiscrete::HandleOptionIncrease);
}

void UGameplaySettingListEntry_SettingDiscrete::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	DiscreteSetting = nullptr;
}

void UGameplaySettingListEntry_SettingDiscrete::HandleOptionDecrease()
{
	//@TODO: Doing this through the UI feels wrong, should use Setting directly.
	Rotator_SettingValue->ShiftTextLeft();
	DiscreteSetting->SetDiscreteOptionByIndex(Rotator_SettingValue->GetSelectedIndex());
}

void UGameplaySettingListEntry_SettingDiscrete::HandleOptionIncrease()
{
	//@TODO: Doing this through the UI feels wrong, should use Setting directly.
	Rotator_SettingValue->ShiftTextRight();
	DiscreteSetting->SetDiscreteOptionByIndex(Rotator_SettingValue->GetSelectedIndex());
}

void UGameplaySettingListEntry_SettingDiscrete::HandleRotatorChangedValue(int32 Value, bool bUserInitiated)
{
	if (bUserInitiated)
	{
		DiscreteSetting->SetDiscreteOptionByIndex(Value);
	}
}

void UGameplaySettingListEntry_SettingDiscrete::HandleEditConditionChanged(UGameplaySetting* InSetting)
{
	Super::HandleEditConditionChanged(InSetting);

	Refresh();
}

//=========================================================
// UGameplaySettingListEntry_SettingScalar
//=========================================================
void UGameplaySettingListEntry_SettingScalar::SetSetting(UGameplaySetting* InSetting)
{
	ScalarSetting = Cast<UGameplaySettingValueScalar>(InSetting);
	
	Super::SetSetting(InSetting);

	Refresh();
}

void UGameplaySettingListEntry_SettingScalar::OnSettingChanged()
{
	Refresh();
}

void UGameplaySettingListEntry_SettingScalar::Refresh()
{
	if (ensure(ScalarSetting))
	{
		const float Value = ScalarSetting->GetValueNormalized();

		Slider_SettingValue->SetValue(Value);
		Slider_SettingValue->SetStepSize(ScalarSetting->GetNormalizedStepSize());
		Text_SettingValue->SetText(ScalarSetting->GetFormattedText());

		TOptional<double> DefaultValue = ScalarSetting->GetDefaultValueNormalized();
		BP_OnDefaultValueChanged(DefaultValue.IsSet() ? DefaultValue.GetValue() : -1.0);

		BP_OnValueChanged(Value);
	}
}

void UGameplaySettingListEntry_SettingScalar::RefreshEditableState(const FGameplaySettingEditableState& InEditableState)
{
	Super::RefreshEditableState(InEditableState);

	const bool bLocalIsEnabled = InEditableState.IsEnabled();
	Slider_SettingValue->SetIsEnabled(bLocalIsEnabled);
	Panel_Value->SetIsEnabled(bLocalIsEnabled);
}

void UGameplaySettingListEntry_SettingScalar::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Slider_SettingValue->OnValueChanged.AddDynamic(this, &UGameplaySettingListEntry_SettingScalar::HandleSliderValueChanged);

	Slider_SettingValue->OnMouseCaptureEnd.AddDynamic(this, &UGameplaySettingListEntry_SettingScalar::HandleSliderCaptureEnded);
	Slider_SettingValue->OnControllerCaptureEnd.AddDynamic(this, &UGameplaySettingListEntry_SettingScalar::HandleSliderCaptureEnded);
}

void UGameplaySettingListEntry_SettingScalar::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	ScalarSetting = nullptr;
}

void UGameplaySettingListEntry_SettingScalar::HandleSliderValueChanged(float Value)
{
	TGuardValue<bool> Suspend(bSuspendChangeUpdates, true);

	if (ensure(ScalarSetting))
	{
		ScalarSetting->SetValueNormalized(Value);
		Value = ScalarSetting->GetValueNormalized();

		Slider_SettingValue->SetValue(Value);
		Text_SettingValue->SetText(ScalarSetting->GetFormattedText());

		BP_OnValueChanged(Value);
	}
}

void UGameplaySettingListEntry_SettingScalar::HandleSliderCaptureEnded()
{
	TGuardValue<bool> Suspend(bSuspendChangeUpdates, true);

	// Commit?
}

//=========================================================
// UGameplaySettingListEntry_SettingAction
//=========================================================
void UGameplaySettingListEntry_SettingAction::SetSetting(UGameplaySetting* InSetting)
{
	Super::SetSetting(InSetting);

	ActionSetting = Cast<UGameplaySettingAction>(InSetting);
	if (ensure(ActionSetting))
	{
		BP_OnSettingAssigned(ActionSetting->GetActionText());
	}
}

void UGameplaySettingListEntry_SettingAction::RefreshEditableState(const FGameplaySettingEditableState& InEditableState)
{
	Super::RefreshEditableState(InEditableState);

	const bool bLocalIsEnabled = InEditableState.IsEnabled();
	Button_Action->SetIsEnabled(bLocalIsEnabled);
}

void UGameplaySettingListEntry_SettingAction::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Action->OnClicked().AddUObject(this, &UGameplaySettingListEntry_SettingAction::HandleActionButtonClicked);
}

void UGameplaySettingListEntry_SettingAction::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	ActionSetting = nullptr;
}

void UGameplaySettingListEntry_SettingAction::HandleActionButtonClicked()
{
	ActionSetting->ExecuteAction();
}

//=========================================================
// UGameplaySettingListEntry_SettingNavigation
//=========================================================
void UGameplaySettingListEntry_SettingNavigation::SetSetting(UGameplaySetting* InSetting)
{
	CollectionSetting = Cast<UGameplaySettingCollectionPage>(InSetting);
	
	Super::SetSetting(InSetting);

	if (ensure(CollectionSetting))
	{
		BP_OnSettingAssigned(CollectionSetting->GetNavigationText());
	}
}

void UGameplaySettingListEntry_SettingNavigation::RefreshEditableState(const FGameplaySettingEditableState& InEditableState)
{
	Super::RefreshEditableState(InEditableState);

	const bool bLocalIsEnabled = InEditableState.IsEnabled();
	Button_Navigate->SetIsEnabled(bLocalIsEnabled);
}

void UGameplaySettingListEntry_SettingNavigation::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	Button_Navigate->OnClicked().AddUObject(this, &UGameplaySettingListEntry_SettingNavigation::HandleNavigationButtonClicked);
}

void UGameplaySettingListEntry_SettingNavigation::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	CollectionSetting = nullptr;
}

void UGameplaySettingListEntry_SettingNavigation::HandleNavigationButtonClicked()
{
	CollectionSetting->ExecuteNavigation();
}

//=========================================================
// UGameplaySettingListEntry_KeyboardInput
//=========================================================

void UGameplaySettingListEntry_KeyboardInput::SetSetting(UGameplaySetting* InSetting)
{
	KeyboardInputSetting = CastChecked<UGameplaySettingValueKeyboard>(InSetting);

	Super::SetSetting(InSetting);

	Refresh();
}

void UGameplaySettingListEntry_KeyboardInput::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Button_PrimaryKey->OnClicked().AddUObject(this, &ThisClass::HandlePrimaryKeyClicked);
	Button_SecondaryKey->OnClicked().AddUObject(this, &ThisClass::HandleSecondaryKeyClicked);
	Button_Clear->OnClicked().AddUObject(this, &ThisClass::HandleClearClicked);
	Button_ResetToDefault->OnClicked().AddUObject(this, &ThisClass::HandleResetToDefaultClicked);	
}

void UGameplaySettingListEntry_KeyboardInput::NativeOnEntryReleased()
{
	Super::NativeOnEntryReleased();

	KeyboardInputSetting = nullptr;
}

void UGameplaySettingListEntry_KeyboardInput::OnSettingChanged()
{
	Refresh();
}

void UGameplaySettingListEntry_KeyboardInput::Refresh()
{
	if (ensure(KeyboardInputSetting))
	{
		Button_PrimaryKey->SetButtonText(KeyboardInputSetting->GetKeyTextFromSlot(EPlayerMappableKeySlot::First));
		Button_SecondaryKey->SetButtonText(KeyboardInputSetting->GetKeyTextFromSlot(EPlayerMappableKeySlot::Second));
		
		// Only display the reset to default button if a mapping is customized
		if (ensure(Button_ResetToDefault))
		{
			if (KeyboardInputSetting->IsMappingCustomized())
			{
				Button_ResetToDefault->SetVisibility(ESlateVisibility::Visible);
			}
			else
			{
				Button_ResetToDefault->SetVisibility(ESlateVisibility::Hidden);
			}
		}		
	}
}

void UGameplaySettingListEntry_KeyboardInput::HandlePrimaryKeyClicked()
{
	UGameplaySettingPressAnyKey* PressAnyKeyPanel = CastChecked<UGameplaySettingPressAnyKey>(UGameplayCommonUILibrary::PushActivatableWidgetForClass(GetOwningPlayer(), GameplayCommonTags::Layer_Modal, PressAnyKeyPanelClass));
	PressAnyKeyPanel->OnKeySelected.AddUObject(this, &ThisClass::HandlePrimaryKeySelected, PressAnyKeyPanel);
	PressAnyKeyPanel->OnKeySelectionCanceled.AddUObject(this, &ThisClass::HandleKeySelectionCanceled, PressAnyKeyPanel);
}

void UGameplaySettingListEntry_KeyboardInput::HandleSecondaryKeyClicked()
{
	UGameplaySettingPressAnyKey* PressAnyKeyPanel = CastChecked<UGameplaySettingPressAnyKey>(UGameplayCommonUILibrary::PushActivatableWidgetForClass(GetOwningPlayer(), GameplayCommonTags::Layer_Modal, PressAnyKeyPanelClass));
	PressAnyKeyPanel->OnKeySelected.AddUObject(this, &ThisClass::HandleSecondaryKeySelected, PressAnyKeyPanel);
	PressAnyKeyPanel->OnKeySelectionCanceled.AddUObject(this, &ThisClass::HandleKeySelectionCanceled, PressAnyKeyPanel);
}

void UGameplaySettingListEntry_KeyboardInput::HandlePrimaryKeySelected(FKey InKey, UGameplaySettingPressAnyKey* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	ChangeBinding(0, InKey);
}

void UGameplaySettingListEntry_KeyboardInput::HandleSecondaryKeySelected(FKey InKey, UGameplaySettingPressAnyKey* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	ChangeBinding(1, InKey);
}

void UGameplaySettingListEntry_KeyboardInput::HandlePrimaryDuplicateKeySelected(FKey InKey, UGameplaySettingKeyAlreadyBound* DuplicateKeyPressAnyKeyPanel) const
{
	DuplicateKeyPressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	KeyboardInputSetting->ChangeBinding(0, OriginalKeyToBind);
}

void UGameplaySettingListEntry_KeyboardInput::HandleSecondaryDuplicateKeySelected(FKey InKey, UGameplaySettingKeyAlreadyBound* DuplicateKeyPressAnyKeyPanel) const
{
	DuplicateKeyPressAnyKeyPanel->OnKeySelected.RemoveAll(this);
	KeyboardInputSetting->ChangeBinding(1, OriginalKeyToBind);
}

void UGameplaySettingListEntry_KeyboardInput::ChangeBinding(int32 BindSlot, const FKey& InKey)
{
	OriginalKeyToBind = InKey;
	TArray<FName> ActionsForKey;
	KeyboardInputSetting->GetAllMappedActionsFromKey(BindSlot, InKey, ActionsForKey);
	if (!ActionsForKey.IsEmpty())
	{
		UGameplaySettingKeyAlreadyBound* KeyAlreadyBoundWarning = CastChecked<UGameplaySettingKeyAlreadyBound>(UGameplayCommonUILibrary::PushActivatableWidgetForClass(GetOwningPlayer(), GameplayCommonTags::Layer_Modal, KeyAlreadyBoundWarningPanelClass));

		FString ActionNames;
		for (FName ActionName : ActionsForKey)
		{
			ActionNames += ActionName.ToString() += ", ";
		}

		FFormatNamedArguments Args;
		Args.Add(TEXT("InKey"), InKey.GetDisplayName());
		Args.Add(TEXT("ActionNames"), FText::FromString(ActionNames));

		KeyAlreadyBoundWarning->SetWarningText(FText::Format(LOCTEXT("WarningText", "{InKey} is already bound to {ActionNames} are you sure you want to rebind it?"), Args));
		KeyAlreadyBoundWarning->SetCancelText(FText::Format(LOCTEXT("CancelText", "Press escape to cancel, or press {InKey} again to confirm rebinding."), Args));

		if (BindSlot == 1)
		{
			KeyAlreadyBoundWarning->OnKeySelected.AddUObject(this, &ThisClass::HandleSecondaryDuplicateKeySelected, KeyAlreadyBoundWarning);
		}
		else
		{
			KeyAlreadyBoundWarning->OnKeySelected.AddUObject(this, &ThisClass::HandlePrimaryDuplicateKeySelected, KeyAlreadyBoundWarning);
		}
		KeyAlreadyBoundWarning->OnKeySelectionCanceled.AddUObject(this, &ThisClass::HandleKeySelectionCanceled, KeyAlreadyBoundWarning);
	}
	else
	{
		KeyboardInputSetting->ChangeBinding(BindSlot, InKey);
	}
}

void UGameplaySettingListEntry_KeyboardInput::HandleKeySelectionCanceled(UGameplaySettingPressAnyKey* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelectionCanceled.RemoveAll(this);
}

void UGameplaySettingListEntry_KeyboardInput::HandleKeySelectionCanceled(UGameplaySettingKeyAlreadyBound* PressAnyKeyPanel)
{
	PressAnyKeyPanel->OnKeySelectionCanceled.RemoveAll(this);
}

void UGameplaySettingListEntry_KeyboardInput::HandleClearClicked()
{
	KeyboardInputSetting->ChangeBinding(0, EKeys::Invalid);
	KeyboardInputSetting->ChangeBinding(1, EKeys::Invalid);
}

void UGameplaySettingListEntry_KeyboardInput::HandleResetToDefaultClicked()
{
	KeyboardInputSetting->ResetToDefault();
}

#undef LOCTEXT_NAMESPACE