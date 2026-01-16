// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayConfirmationDialog.h"
#include "Widgets/GameplayButtonBase.h"
#include "CommonRichTextBlock.h"
#include "Components/DynamicEntryBox.h"
#include "Misc/GameplayCommonTypes.h"

#define LOCTEXT_NAMESPACE "GameplayConfirmationDialog"

//===================================================
// UGameplayConfirmationDialogDescriptor
//===================================================
UGameplayConfirmationDescriptor::UGameplayConfirmationDescriptor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

UGameplayConfirmationDescriptor* UGameplayConfirmationDescriptor::CreateConfirmationOk(const FText& InDialogTitle, const FText& InDialogMessage)
{
	UGameplayConfirmationDescriptor* Descriptor = NewObject<UGameplayConfirmationDescriptor>();
	Descriptor->DialogTitle = InDialogTitle;
	Descriptor->DialogMessage = InDialogMessage;

	FGameplayConfirmationDialogAction OkAction;
	OkAction.DialogResult = EGameplayConfirmationResult::Confirmed;
	OkAction.ButtonDisplayText = LOCTEXT("OkDialogButton_Title", "Ok");

	Descriptor->DialogButtons.Add(OkAction);

	return Descriptor;
}

UGameplayConfirmationDescriptor* UGameplayConfirmationDescriptor::CreateConfirmationOkCancel(const FText& InDialogTitle, const FText& InDialogMessage)
{
	UGameplayConfirmationDescriptor* Descriptor = NewObject<UGameplayConfirmationDescriptor>();
	Descriptor->DialogTitle = InDialogTitle;
	Descriptor->DialogMessage = InDialogMessage;

	FGameplayConfirmationDialogAction OkAction;
	OkAction.DialogResult = EGameplayConfirmationResult::Confirmed;
	OkAction.ButtonDisplayText = LOCTEXT("OkDialogButton_Title", "Ok");

	FGameplayConfirmationDialogAction CancelAction;
	CancelAction.DialogResult = EGameplayConfirmationResult::Cancelled;
	CancelAction.ButtonDisplayText = LOCTEXT("CancelDialogButton_Title", "Cancel");

	Descriptor->DialogButtons.Add(OkAction);
	Descriptor->DialogButtons.Add(CancelAction);

	return Descriptor;
}

UGameplayConfirmationDescriptor* UGameplayConfirmationDescriptor::CreateConfirmationYes(const FText& InDialogTitle, const FText& InDialogMessage)
{
	UGameplayConfirmationDescriptor* Descriptor = NewObject<UGameplayConfirmationDescriptor>();
	Descriptor->DialogTitle = InDialogTitle;
	Descriptor->DialogMessage = InDialogMessage;

	FGameplayConfirmationDialogAction YesAction;
	YesAction.DialogResult = EGameplayConfirmationResult::Confirmed;
	YesAction.ButtonDisplayText = LOCTEXT("YesDialogButton_Title", "Yes");

	Descriptor->DialogButtons.Add(YesAction);

	return Descriptor;
}

UGameplayConfirmationDescriptor* UGameplayConfirmationDescriptor::CreateConfirmationYesNo(const FText& InDialogTitle, const FText& InDialogMessage)
{
	UGameplayConfirmationDescriptor* Descriptor = NewObject<UGameplayConfirmationDescriptor>();
	Descriptor->DialogTitle = InDialogTitle;
	Descriptor->DialogMessage = InDialogMessage;

	FGameplayConfirmationDialogAction YesAction;
	YesAction.DialogResult = EGameplayConfirmationResult::Confirmed;
	YesAction.ButtonDisplayText = LOCTEXT("YesDialogButton_Title", "Yes");
	
	FGameplayConfirmationDialogAction NoAction;
	NoAction.DialogResult = EGameplayConfirmationResult::Declined;
	NoAction.ButtonDisplayText = LOCTEXT("NoDialogButton_Title", "No");

	Descriptor->DialogButtons.Add(YesAction);
	Descriptor->DialogButtons.Add(NoAction);

	return Descriptor;
}

UGameplayConfirmationDescriptor* UGameplayConfirmationDescriptor::CreateConfirmationYesNoCancel(const FText& InDialogTitle, const FText& InDialogMessage)
{
	UGameplayConfirmationDescriptor* Descriptor = NewObject<UGameplayConfirmationDescriptor>();
	Descriptor->DialogTitle = InDialogTitle;
	Descriptor->DialogMessage = InDialogMessage;

	FGameplayConfirmationDialogAction YesAction;
	YesAction.DialogResult = EGameplayConfirmationResult::Confirmed;
	YesAction.ButtonDisplayText = LOCTEXT("YesDialogButton_Title", "Yes");

	FGameplayConfirmationDialogAction NoAction;
	NoAction.DialogResult = EGameplayConfirmationResult::Declined;
	NoAction.ButtonDisplayText = LOCTEXT("NoDialogButton_Title", "No");

	FGameplayConfirmationDialogAction CancelAction;
	CancelAction.DialogResult = EGameplayConfirmationResult::Cancelled;
	CancelAction.ButtonDisplayText = LOCTEXT("CancelDialogButton_Title", "Cancel");

	Descriptor->DialogButtons.Add(YesAction);
	Descriptor->DialogButtons.Add(NoAction);
	Descriptor->DialogButtons.Add(CancelAction);

	return Descriptor;
}

//===================================================
// UGameplayConfirmationDialog
//===================================================
UGameplayConfirmationDialog::UGameplayConfirmationDialog(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	InputMode = EGameplayActivatableInputMode::Menu;
}

UWidget* UGameplayConfirmationDialog::NativeGetDesiredFocusTarget() const
{
	// Set focus on the last button so Gamepad can start with.
	if (EntryBox_DialogButtons->GetNumEntries() != 0)
	{
		EntryBox_DialogButtons->GetAllEntries().Last()->SetFocus();
	}

	return Super::NativeGetDesiredFocusTarget();
}

void UGameplayConfirmationDialog::SetupDialog(UGameplayConfirmationDescriptor* Descriptor, FGameplayConfirmationDialogResultSignature ResultCallback)
{
	check(Descriptor && CommonRichText_DialogTitle && CommonRichText_DialogMessage);

	CommonRichText_DialogTitle->SetText(Descriptor->DialogTitle);
	CommonRichText_DialogMessage->SetText(Descriptor->DialogMessage);

	// Checking if the entry box has old buttons created previously.
	if (EntryBox_DialogButtons->GetNumEntries() != 0)
	{
		// Clearing the old buttons the entry box has. The widget type for the entry box
		// is specified in the child widget blueprint.
		EntryBox_DialogButtons->Reset<UGameplayButtonBase>([](const UGameplayButtonBase& ExistingButton)
		{
			ExistingButton.OnClicked().Clear();
		});
	}

	check(!Descriptor->DialogButtons.IsEmpty());

	for (const FGameplayConfirmationDialogAction& DialogButton : Descriptor->DialogButtons)
	{
		UGameplayButtonBase* Button = EntryBox_DialogButtons->CreateEntry<UGameplayButtonBase>();
		Button->SetButtonText(DialogButton.ButtonDisplayText);
		Button->OnClicked().AddUObject(this, &UGameplayConfirmationDialog::NativeOnConfirmationDialogButtonClicked, DialogButton.DialogResult);
	}

	OnResultCallback = ResultCallback;

	if (EntryBox_DialogButtons->GetNumEntries() != 0)
	{
		// Set focus on the last button. So if there are two buttons, one is yes, one is no.
		// Our gamepad will focus on the No button (last button added).
		EntryBox_DialogButtons->GetAllEntries().Last()->SetFocus();
	}
}

void UGameplayConfirmationDialog::KillDialog()
{
}

void UGameplayConfirmationDialog::NativeOnConfirmationDialogButtonClicked(EGameplayConfirmationResult Result)
{
	OnResultCallback.ExecuteIfBound(Result);
	BP_OnConfirmationDialogButtonClicked(Result);
}

void UGameplayConfirmationDialog::BP_OnConfirmationDialogButtonClicked_Implementation(EGameplayConfirmationResult Result)
{
	DeactivateWidget();
}

#undef LOCTEXT_NAMESPACE
