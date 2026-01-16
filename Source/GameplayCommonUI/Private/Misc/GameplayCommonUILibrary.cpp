// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Misc/GameplayCommonUILibrary.h"
#include "Framework/GameplayCommonUIPolicy.h"
#include "Framework/GameplayCommonUISettings.h"
#include "CommonInputSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Subsystems/GameplayCommonUISubsystem.h"
#include "Widgets/GameplayPrimaryLayout.h"
#include "CommonActivatableWidget.h"

int32 UGameplayCommonUILibrary::InputSuspensions = 0;

ECommonInputType UGameplayCommonUILibrary::GetOwningPlayerInputType(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType();
		}
	}

	return ECommonInputType::Count;
}

bool UGameplayCommonUILibrary::IsOwningPlayerUsingTouch(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Touch;
		}
	}
	return false;
}

bool UGameplayCommonUILibrary::IsOwningPlayerUsingGamepad(const UUserWidget* WidgetContextObject)
{
	if (WidgetContextObject)
	{
		if (const UCommonInputSubsystem* InputSubsystem = UCommonInputSubsystem::Get(WidgetContextObject->GetOwningLocalPlayer()))
		{
			return InputSubsystem->GetCurrentInputType() == ECommonInputType::Gamepad;
		}
	}
	return false;
}

void UGameplayCommonUILibrary::PopSingleWidget(UCommonActivatableWidget* ActivatableWidget)
{
	if (!ActivatableWidget)
	{
		// Ignore request to pop an already deleted widget.
		return;
	}

	const ULocalPlayer* LocalPlayer = ActivatableWidget->GetOwningLocalPlayer();
	if (!IsValid(LocalPlayer)) return;

	const UGameplayCommonUISubsystem* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UGameplayCommonUISubsystem>();
	if (!UIManager) return;

	const UGameplayCommonUIPolicy* UIPolicy = UIManager->GetUIPolicy();
	if (!IsValid(UIPolicy)) return;

	UGameplayPrimaryLayout* RootLayout = UIPolicy->GetPrimaryLayoutFromLocalPlayer(LocalPlayer);
	if (!IsValid(RootLayout)) return;

	RootLayout->FindAndRemoveWidgetFromLayer(ActivatableWidget);
}

void UGameplayCommonUILibrary::PopWidgetsFromLayer(const APlayerController* OwningPlayer, FGameplayTag LayerTag)
{
	if (!IsValid(OwningPlayer)) return;

	const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!IsValid(LocalPlayer)) return;

	const UGameplayCommonUISubsystem* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UGameplayCommonUISubsystem>();
	if (!UIManager) return;

	const UGameplayCommonUIPolicy* UIPolicy = UIManager->GetUIPolicy();
	if (!IsValid(UIPolicy)) return;

	UGameplayPrimaryLayout* RootLayout = UIPolicy->GetPrimaryLayoutFromLocalPlayer(LocalPlayer);
	if (!IsValid(RootLayout)) return;

	RootLayout->FindAndRemoveWidgetsFromLayer(LayerTag);
}

void UGameplayCommonUILibrary::PopWidgetsFromLayout(APlayerController* OwningPlayer)
{
	if (!IsValid(OwningPlayer)) return;

	const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!IsValid(LocalPlayer)) return;

	const UGameplayCommonUISubsystem* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UGameplayCommonUISubsystem>();
	if (!UIManager) return;

	const UGameplayCommonUIPolicy* UIPolicy = UIManager->GetUIPolicy();
	if (!IsValid(UIPolicy)) return;

	UGameplayPrimaryLayout* RootLayout = UIPolicy->GetPrimaryLayoutFromLocalPlayer(LocalPlayer);
	if (!IsValid(RootLayout)) return;

	RootLayout->ClearAllLayerStacks();
}

UCommonActivatableWidget* UGameplayCommonUILibrary::PushActivatableWidgetForClass(const APlayerController* OwningPlayer, FGameplayTag LayerTag, TSubclassOf<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(OwningPlayer) || !ensure(WidgetClass != nullptr))
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!IsValid(LocalPlayer)) return nullptr;

	const UGameplayCommonUISubsystem* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UGameplayCommonUISubsystem>();
	if (!UIManager) return nullptr;

	const UGameplayCommonUIPolicy* UIPolicy = UIManager->GetUIPolicy();
	if (!IsValid(UIPolicy)) return nullptr;

	UGameplayPrimaryLayout* RootLayout = UIPolicy->GetPrimaryLayoutFromLocalPlayer(LocalPlayer);
	if (!IsValid(RootLayout)) return nullptr;

	return RootLayout->PushWidgetToLayerStack(LayerTag, WidgetClass);
}

void UGameplayCommonUILibrary::PushStreamedActivatableWidgetForClass(const APlayerController* OwningPlayer, FGameplayTag LayerTag, TSoftClassPtr<UCommonActivatableWidget> WidgetClass)
{
	if (!ensure(OwningPlayer) || !ensure(!WidgetClass.IsNull()))
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!IsValid(LocalPlayer)) return;

	const UGameplayCommonUISubsystem* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UGameplayCommonUISubsystem>();
	if (!UIManager) return;

	const UGameplayCommonUIPolicy* UIPolicy = UIManager->GetUIPolicy();
	if (!IsValid(UIPolicy)) return;

	UGameplayPrimaryLayout* RootLayout = UIPolicy->GetPrimaryLayoutFromLocalPlayer(LocalPlayer);
	if (!IsValid(RootLayout)) return;

	constexpr bool bSuspendInputUntilComplete = true;
	RootLayout->PushWidgetToLayerStackAsync(LayerTag, bSuspendInputUntilComplete, WidgetClass);
}

UCommonActivatableWidget* UGameplayCommonUILibrary::PushActivatableWidgetForTag(const APlayerController* OwningPlayer, FGameplayTag LayerTag, FGameplayTag WidgetTag)
{
	if (!ensure(OwningPlayer) || !ensure(WidgetTag.IsValid()))
	{
		return nullptr;
	}

	const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!IsValid(LocalPlayer)) return nullptr;

	const UGameplayCommonUISubsystem* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UGameplayCommonUISubsystem>();
	if (!UIManager) return nullptr;

	const UGameplayCommonUIPolicy* UIPolicy = UIManager->GetUIPolicy();
	if (!IsValid(UIPolicy)) return nullptr;

	UGameplayPrimaryLayout* RootLayout = UIPolicy->GetPrimaryLayoutFromLocalPlayer(LocalPlayer);
	if (!IsValid(RootLayout)) return nullptr;

	const UGameplayCommonUISettings* Settings = GetDefault<UGameplayCommonUISettings>();
	const TSoftClassPtr<UCommonActivatableWidget> FoundWidget = Settings->RegisteredActivatableWidgets.FindRef(WidgetTag);
	
	return RootLayout->PushWidgetToLayerStack(LayerTag, FoundWidget.LoadSynchronous());
}

void UGameplayCommonUILibrary::PushStreamedActivatableWidgetForTag(const APlayerController* OwningPlayer, FGameplayTag LayerTag, FGameplayTag WidgetTag)
{
	if (!ensure(OwningPlayer) || !ensure(WidgetTag.IsValid()))
	{
		return;
	}

	const ULocalPlayer* LocalPlayer = OwningPlayer->GetLocalPlayer();
	if (!IsValid(LocalPlayer)) return;

	const UGameplayCommonUISubsystem* UIManager = LocalPlayer->GetGameInstance()->GetSubsystem<UGameplayCommonUISubsystem>();
	if (!UIManager) return;

	const UGameplayCommonUIPolicy* UIPolicy = UIManager->GetUIPolicy();
	if (!IsValid(UIPolicy)) return;

	UGameplayPrimaryLayout* RootLayout = UIPolicy->GetPrimaryLayoutFromLocalPlayer(LocalPlayer);
	if (!IsValid(RootLayout)) return;

	const UGameplayCommonUISettings* Settings = GetDefault<UGameplayCommonUISettings>();
	const TSoftClassPtr<UCommonActivatableWidget> FoundWidget = Settings->RegisteredActivatableWidgets.FindRef(WidgetTag);

	constexpr bool bSuspendInputUntilComplete = true;
	RootLayout->PushWidgetToLayerStackAsync(LayerTag, bSuspendInputUntilComplete, FoundWidget.LoadSynchronous());
}

ULocalPlayer* UGameplayCommonUILibrary::GetLocalPlayerFromController(const APlayerController* PlayerController)
{
	if (PlayerController)
	{
		return Cast<ULocalPlayer>(PlayerController->Player);
	}

	return nullptr;
}

FName UGameplayCommonUILibrary::SuspendInputForPlayer(APlayerController* PlayerController, FName SuspendReason)
{
	return SuspendInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendReason);
}

FName UGameplayCommonUILibrary::SuspendInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendReason)
{
	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		InputSuspensions++;
		FName SuspendToken = SuspendReason;
		SuspendToken.SetNumber(InputSuspensions);

		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendToken, true);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendToken, true);

		return SuspendToken;
	}

	return NAME_None;
}

void UGameplayCommonUILibrary::ResumeInputForPlayer(APlayerController* PlayerController, FName SuspendReason)
{
	ResumeInputForPlayer(PlayerController ? PlayerController->GetLocalPlayer() : nullptr, SuspendReason);
}

void UGameplayCommonUILibrary::ResumeInputForPlayer(const ULocalPlayer* LocalPlayer, FName SuspendReason)
{
	if (SuspendReason == NAME_None)
	{
		return;
	}

	if (UCommonInputSubsystem* CommonInputSubsystem = UCommonInputSubsystem::Get(LocalPlayer))
	{
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::MouseAndKeyboard, SuspendReason, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Gamepad, SuspendReason, false);
		CommonInputSubsystem->SetInputTypeFilter(ECommonInputType::Touch, SuspendReason, false);
	}
}

void UGameplayCommonUILibrary::OnPrimaryLayoutReady(const APlayerController* OwningPlayer, TFunction<void(UGameplayPrimaryLayout*)> Callback)
{
	if (UGameplayPrimaryLayout* Layout = UGameplayPrimaryLayout::GetPrimaryGameLayout(OwningPlayer))
	{
		Callback(Layout);
	}
	else if (OwningPlayer)
	{
		if (UGameplayCommonUISubsystem* UIManager = UGameplayCommonUISubsystem::Get(OwningPlayer))
		{
			UIManager->OnPrimaryLayoutSet.AddWeakLambda(const_cast<APlayerController*>(OwningPlayer), [Callback](UGameplayPrimaryLayout* NewLayout)
			{
				Callback(NewLayout);
			});
		}
	}
}

