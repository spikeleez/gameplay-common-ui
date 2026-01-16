// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Async/GameplayShowConfirmationDialogAsync.h"
#include "Widgets/GameplayConfirmationDialog.h"
#include "Framework/GameplayCommonUISettings.h"
#include "Subsystems/GameplayCommonLocalPlayerSubsystem.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"

UGameplayShowConfirmationDialogAsync::UGameplayShowConfirmationDialogAsync(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}

UGameplayShowConfirmationDialogAsync* UGameplayShowConfirmationDialogAsync::ShowConfirmationYes(UObject* WorldContext, FText Title, FText Message, bool bShowError)
{
	UGameplayShowConfirmationDialogAsync* Action = NewObject<UGameplayShowConfirmationDialogAsync>();
	Action->WorldContextObject = WorldContext;
	Action->Descriptor = UGameplayConfirmationDescriptor::CreateConfirmationYes(Title, Message);
	Action->bIsErrorDialog = bShowError;
	Action->RegisterWithGameInstance(WorldContext);

	return Action;
}

UGameplayShowConfirmationDialogAsync* UGameplayShowConfirmationDialogAsync::ShowConfirmationYesNo(UObject* WorldContext, FText Title, FText Message, bool bShowError)
{
	UGameplayShowConfirmationDialogAsync* Action = NewObject<UGameplayShowConfirmationDialogAsync>();
	Action->WorldContextObject = WorldContext;
	Action->Descriptor = UGameplayConfirmationDescriptor::CreateConfirmationYesNo(Title, Message);
	Action->bIsErrorDialog = bShowError;
	Action->RegisterWithGameInstance(WorldContext);

	return Action;
}

UGameplayShowConfirmationDialogAsync* UGameplayShowConfirmationDialogAsync::ShowConfirmationYesNoCancel(UObject* WorldContext, FText Title, FText Message, bool bShowError)
{
	UGameplayShowConfirmationDialogAsync* Action = NewObject<UGameplayShowConfirmationDialogAsync>();
	Action->WorldContextObject = WorldContext;
	Action->Descriptor = UGameplayConfirmationDescriptor::CreateConfirmationYesNoCancel(Title, Message);
	Action->bIsErrorDialog = bShowError;
	Action->RegisterWithGameInstance(WorldContext);

	return Action;
}

UGameplayShowConfirmationDialogAsync* UGameplayShowConfirmationDialogAsync::ShowConfirmationOk(UObject* WorldContext, FText Title, FText Message, bool bShowError)
{
	UGameplayShowConfirmationDialogAsync* Action = NewObject<UGameplayShowConfirmationDialogAsync>();
	Action->WorldContextObject = WorldContext;
	Action->Descriptor = UGameplayConfirmationDescriptor::CreateConfirmationOk(Title, Message);
	Action->bIsErrorDialog = bShowError;
	Action->RegisterWithGameInstance(WorldContext);

	return Action;
}

UGameplayShowConfirmationDialogAsync* UGameplayShowConfirmationDialogAsync::ShowConfirmationOkCancel(UObject* WorldContext, FText Title, FText Message, bool bShowError)
{
	UGameplayShowConfirmationDialogAsync* Action = NewObject<UGameplayShowConfirmationDialogAsync>();
	Action->WorldContextObject = WorldContext;
	Action->Descriptor = UGameplayConfirmationDescriptor::CreateConfirmationOkCancel(Title, Message);
	Action->bIsErrorDialog = bShowError;
	Action->RegisterWithGameInstance(WorldContext);

	return Action;
}

UGameplayShowConfirmationDialogAsync* UGameplayShowConfirmationDialogAsync::ShowConfirmationCustom(UObject* WorldContext, FGameplayTag DialogTag, bool bShowError)
{
	const UGameplayCommonUISettings* Settings = GetDefault<UGameplayCommonUISettings>();
	check(Settings);

	const TSoftClassPtr<UGameplayConfirmationDescriptor> Descriptor = Settings->RegisteredDialogDescriptors.FindRef(DialogTag);
	if (Descriptor.IsNull())
	{
		return nullptr;
	}

	const TSubclassOf<UGameplayConfirmationDescriptor> ConcreteDescriptor = Descriptor.LoadSynchronous();
	if (!ConcreteDescriptor.Get())
	{
		return nullptr;
	}

	UGameplayShowConfirmationDialogAsync* Action = NewObject<UGameplayShowConfirmationDialogAsync>();
	Action->WorldContextObject = WorldContext;
	Action->Descriptor = ConcreteDescriptor->GetDefaultObject<UGameplayConfirmationDescriptor>();
	Action->bIsErrorDialog = bShowError;
	Action->RegisterWithGameInstance(WorldContext);

	return Action;
}

void UGameplayShowConfirmationDialogAsync::Activate()
{
	if (WorldContextObject && !TargetLocalPlayer)
	{
		if (const UUserWidget* UserWidget = Cast<UUserWidget>(WorldContextObject))
		{
			TargetLocalPlayer = UserWidget->GetOwningLocalPlayer<ULocalPlayer>();
		}
		else if (const APlayerController* PC = Cast<APlayerController>(WorldContextObject))
		{
			TargetLocalPlayer = PC->GetLocalPlayer();
		}
		else if (const UWorld* World = WorldContextObject->GetWorld())
		{
			if (const UGameInstance* GameInstance = World->GetGameInstance<UGameInstance>())
			{
				TargetLocalPlayer = GameInstance->GetPrimaryPlayerController(false)->GetLocalPlayer();
			}
		}
	}

	if (TargetLocalPlayer)
	{
		if (UGameplayCommonLocalPlayerSubsystem* UILocalPlayerSubsystem = TargetLocalPlayer->GetSubsystem<UGameplayCommonLocalPlayerSubsystem>())
		{
			const FGameplayConfirmationDialogResultSignature ResultCallback = FGameplayConfirmationDialogResultSignature::CreateUObject(this, &ThisClass::HandleConfirmationResult);
			if (bIsErrorDialog)
			{
				UILocalPlayerSubsystem->ShowError(Descriptor, ResultCallback);
			}
			else
			{
				UILocalPlayerSubsystem->ShowConfirmation(Descriptor, ResultCallback);
			}
			return;
		}
	}

	// If we couldn't make the confirmation, just handle an unknown result and broadcast nothing
	HandleConfirmationResult(EGameplayConfirmationResult::Unknown);
}

void UGameplayShowConfirmationDialogAsync::HandleConfirmationResult(EGameplayConfirmationResult Result)
{
	DialogResult.Broadcast(Result);
	SetReadyToDestroy();
}
