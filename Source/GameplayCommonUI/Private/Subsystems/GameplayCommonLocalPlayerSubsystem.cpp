// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Subsystems/GameplayCommonLocalPlayerSubsystem.h"
#include "Framework/GameplayCommonUISettings.h"
#include "Misc/GameplayCommonTags.h"
#include "Framework/GameplayCommonUIPolicy.h"
#include "GameFramework/PlayerState.h"
#include "TimerManager.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Widgets/GameplayConfirmationDialog.h"

bool UGameplayCommonLocalPlayerSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<ULocalPlayer>(Outer)->GetGameInstance()->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// Only create an instance if there is no override implementation defined elsewhere.
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UGameplayCommonLocalPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGameplayCommonLocalPlayerSubsystem::Deinitialize()
{
	if (const UGameInstance* GameInstance = GetLocalPlayer()->GetGameInstance())
	{
		if (const UGameplayCommonUISubsystem* UIManagerSubsystem = GameInstance->GetSubsystem<UGameplayCommonUISubsystem>())
		{
			if (UGameplayCommonUIPolicy* Policy = UIManagerSubsystem->GetUIPolicy())
			{
				Policy->NotifyPlayerDestroyed(GetLocalPlayer());
			}
		}
	}
	
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearAllTimersForObject(this);
	}
	
	Super::Deinitialize();
}

void UGameplayCommonLocalPlayerSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);
	
	// Stop any active timer from a previous controller to prevent conflicts.
	if (const UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(TimerHandle_CheckPlayerState);
	}

	// If the controller is being removed (nullptr), we just return after clearing the timer.
	if (!NewPlayerController)
	{
		WeakPlayerController = nullptr;
		return;
	}

	// Store a weak reference to the new controller for safe async access.
	WeakPlayerController = NewPlayerController;

	// Check if the PlayerState is already valid (Common on Server or Standalone).
	if (NewPlayerController->PlayerState)
	{
		OnLocalPlayerStateSet.Broadcast(NewPlayerController->PlayerState);
	}
	else
	{
		// PlayerState is not ready yet (Common on Client). Start a safe polling timer.
		if (const UWorld* World = GetWorld())
		{
			// Checking every 0.1s is responsive enough for UI without hitting performance.
			constexpr float CheckInterval = 0.1f;
			constexpr bool bLoop = true;
			World->GetTimerManager().SetTimer(TimerHandle_CheckPlayerState, this, &ThisClass::CheckPlayerStateValidity, CheckInterval, bLoop);
		}
	}
	
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle_CheckPlayerState);
		
	if (NewPlayerController->PlayerState)
	{
		OnLocalPlayerStateSet.Broadcast(NewPlayerController->PlayerState);
	}
	else
	{
		GetWorld()->GetTimerManager().SetTimer(TimerHandle_CheckPlayerState, this, &ThisClass::CheckPlayerStateValidity, 0.1f, true);
	}
		
	NewPlayerController->GetOnNewPawnNotifier().AddUObject(this, &ThisClass::HandlePawnChanged);
		
	if (const UGameplayCommonUISubsystem* UIManagerSubsystem = UGameplayCommonUISubsystem::Get(NewPlayerController))
	{
		if (UGameplayCommonUIPolicy* Policy = UIManagerSubsystem->GetUIPolicy())
		{
			Policy->NotifyPlayerAdded(GetLocalPlayer());
		}
	}
		
	if (APawn* CurrentControlledPawn = NewPlayerController->GetPawn())
	{
		HandlePawnChanged(CurrentControlledPawn);
	}
}

void UGameplayCommonLocalPlayerSubsystem::ShowConfirmation(UGameplayConfirmationDescriptor* Descriptor, FGameplayConfirmationDialogResultSignature ResultCallback)
{
	const UGameplayCommonUISettings* Settings = GetDefault<UGameplayCommonUISettings>();
	check(Settings);

	const TSubclassOf<UGameplayConfirmationDialog> ConfirmationDialogClass = Settings->ConfirmationDialogClass.LoadSynchronous();
	if (ensure(ConfirmationDialogClass))
	{
		UGameplayPrimaryLayout* PrimaryLayout = UGameplayPrimaryLayout::GetPrimaryGameLayout(GetLocalPlayer());
		if (IsValid(PrimaryLayout))
		{
			PrimaryLayout->PushWidgetToLayerStack<UGameplayConfirmationDialog>(GameplayCommonTags::Layer_Modal, ConfirmationDialogClass, 
			[Descriptor, ResultCallback](UGameplayConfirmationDialog& Dialog)
			{
				Dialog.SetupDialog(Descriptor, ResultCallback);
			});
		}
	}
}

void UGameplayCommonLocalPlayerSubsystem::ShowError(UGameplayConfirmationDescriptor* Descriptor, FGameplayConfirmationDialogResultSignature ResultCallback)
{
	const UGameplayCommonUISettings* Settings = GetDefault<UGameplayCommonUISettings>();
	check(Settings);

	const TSubclassOf<UGameplayConfirmationDialog> ErrorDialogClass = Settings->ErrorDialogClass.LoadSynchronous();
	if (ensure(ErrorDialogClass))
	{
		UGameplayPrimaryLayout* PrimaryLayout = UGameplayPrimaryLayout::GetPrimaryGameLayout(GetLocalPlayer());
		if (IsValid(PrimaryLayout))
		{
			PrimaryLayout->PushWidgetToLayerStack<UGameplayConfirmationDialog>(GameplayCommonTags::Layer_Modal, ErrorDialogClass, 
			[Descriptor, ResultCallback](UGameplayConfirmationDialog& Dialog)
			{
				Dialog.SetupDialog(Descriptor, ResultCallback);
			});
		}
	}
}

void UGameplayCommonLocalPlayerSubsystem::HandlePawnChanged(APawn* NewPawn)
{
	if (OnLocalPlayerPawnSet.IsBound())
	{
		OnLocalPlayerPawnSet.Broadcast(NewPawn);
	}
}

void UGameplayCommonLocalPlayerSubsystem::CheckPlayerStateValidity()
{
	// Safety Check: If the controller was destroyed while waiting, stop the timer.
	if (!WeakPlayerController.IsValid())
	{
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TimerHandle_CheckPlayerState);
		}
		return;
	}
	
	const APlayerController* ActualPlayerController = WeakPlayerController.Get();
	
	// Resolve the weak pointer to access the controller safely.
	if (ActualPlayerController && ActualPlayerController->PlayerState)
	{
		// SUCCESS: Stop the timer immediately.
		if (const UWorld* World = GetWorld())
		{
			World->GetTimerManager().ClearTimer(TimerHandle_CheckPlayerState);
		}
		
		// Broadcast the event to listeners.
		OnLocalPlayerStateSet.Broadcast(ActualPlayerController->PlayerState);
		
		// Clear the weak reference as we don't need it for the timer anymore.
		WeakPlayerController = nullptr;
	}
}
