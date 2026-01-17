// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplayCommonUIPolicy.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/GameplayPrimaryLayout.h"
#include "Subsystems/GameplayCommonUISubsystem.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/Engine.h"
#include "Engine/GameInstance.h"

DEFINE_LOG_CATEGORY_STATIC(LogGameplayUIPolicy, Log, All);

UGameplayCommonUIPolicy::UGameplayCommonUIPolicy(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	static ConstructorHelpers::FClassFinder<UGameplayPrimaryLayout> DefaultPrimaryLayoutClass(TEXT("/GameplayCommonUI/Widgets/Foundation/GameplayPrimaryLayout"));
	if (DefaultPrimaryLayoutClass.Succeeded())
	{
		PrimaryLayoutClass = DefaultPrimaryLayoutClass.Class;
	}
}

UWorld* UGameplayCommonUIPolicy::GetWorld() const
{
	return GetOwningSubsystem()->GetGameInstance()->GetWorld();
}

const UGameplayCommonUIPolicy* UGameplayCommonUIPolicy::GetUIPolicy(const UObject* WorldContextObject)
{
	const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::Assert);

	const UGameInstance* GameInstance = World->GetGameInstance();
	check(GameInstance);

	const UGameplayCommonUISubsystem* UIManagerSubsystem = UGameInstance::GetSubsystem<UGameplayCommonUISubsystem>(GameInstance);
	check(UIManagerSubsystem);

	return UIManagerSubsystem->GetUIPolicy();
}

UGameplayCommonUISubsystem* UGameplayCommonUIPolicy::GetOwningSubsystem() const
{
	return CastChecked<UGameplayCommonUISubsystem>(GetOuter());
}

UGameplayPrimaryLayout* UGameplayCommonUIPolicy::GetPrimaryLayoutFromLocalPlayer(const ULocalPlayer* LocalPlayer) const
{
	const FGameplayPrimaryLayoutViewport* LayoutInfo = PrimaryViewportLayouts.FindByKey(LocalPlayer);
	return LayoutInfo ? LayoutInfo->PrimaryLayout : nullptr;
}

void UGameplayCommonUIPolicy::NotifyPawnChanged(ULocalPlayer* LocalPlayer, APawn* NewPawn)
{
	RemovePrimaryLayoutFromViewport(LocalPlayer, GetPrimaryLayoutFromLocalPlayer(LocalPlayer));
	CreatePrimaryLayout(LocalPlayer);
}

TSubclassOf<UGameplayPrimaryLayout> UGameplayCommonUIPolicy::GetPrimaryLayoutClass()
{
	return PrimaryLayoutClass;
}

UGameplayPrimaryLayout* UGameplayCommonUIPolicy::GetPrimaryLayout() const
{
	return PrimaryLayout;
}

void UGameplayCommonUIPolicy::AddPrimaryLayoutToViewport(ULocalPlayer* LocalPlayer, UGameplayPrimaryLayout* Layout)
{
	UE_LOG(LogGameplayUIPolicy, Log, TEXT("[%s] is adding player [%s]'s root layout [%s] to the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

	Layout->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	Layout->AddToPlayerScreen(1000);

	OnPrimaryLayoutAddedToViewport(LocalPlayer, Layout);
}

void UGameplayCommonUIPolicy::RemovePrimaryLayoutFromViewport(ULocalPlayer* LocalPlayer, UGameplayPrimaryLayout* Layout)
{	
	const TWeakPtr<SWidget> LayoutSlateWidget = Layout->GetCachedWidget();
	if (LayoutSlateWidget.IsValid())
	{
		UE_LOG(LogGameplayUIPolicy, Log, TEXT("[%s] is removing player [%s]'s root layout [%s] from the viewport"), *GetName(), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));

		Layout->RemoveFromParent();
		if (LayoutSlateWidget.IsValid())
		{
			UE_LOG(LogGameplayUIPolicy, Log, TEXT("Player [%s]'s root layout [%s] has been removed from the viewport, but other references to its underlying Slate widget still exist. Noting in case we leak it."), *GetNameSafe(LocalPlayer), *GetNameSafe(Layout));
		}

		OnPrimaryLayoutRemovedFromViewport(LocalPlayer, Layout);
	}
}

void UGameplayCommonUIPolicy::OnPrimaryLayoutAddedToViewport(ULocalPlayer* LocalPlayer, UGameplayPrimaryLayout* Layout)
{
#if WITH_EDITOR
	if (GIsEditor && LocalPlayer->IsPrimaryPlayer())
	{
		// So our controller will work in PIE without needing to click in the viewport
		FSlateApplication::Get().SetUserFocusToGameViewport(0);
	}
#endif
}

void UGameplayCommonUIPolicy::OnPrimaryLayoutRemovedFromViewport(ULocalPlayer* LocalPlayer, UGameplayPrimaryLayout* Layout)
{
	// ...
}

void UGameplayCommonUIPolicy::OnPrimaryLayoutReleased(ULocalPlayer* LocalPlayer, UGameplayPrimaryLayout* Layout)
{
	// ...
}

void UGameplayCommonUIPolicy::CreatePrimaryLayout(ULocalPlayer* LocalPlayer)
{
	if (APlayerController* PlayerController = LocalPlayer->GetPlayerController(GetWorld()))
	{
		const TSubclassOf<UGameplayPrimaryLayout> LayoutWidgetClass = GetPrimaryLayoutClass();
		if (ensure(LayoutWidgetClass && !LayoutWidgetClass->HasAnyClassFlags(CLASS_Abstract)))
		{
			UGameplayPrimaryLayout* NewLayoutObject = CreateWidget<UGameplayPrimaryLayout>(PlayerController, LayoutWidgetClass);
			PrimaryViewportLayouts.Emplace(LocalPlayer, NewLayoutObject, true);

			AddPrimaryLayoutToViewport(LocalPlayer, NewLayoutObject);
			
			if (LocalPlayer->IsPrimaryPlayer())
			{
				PrimaryLayout = NewLayoutObject;
			}

			GetOwningSubsystem()->OnPrimaryLayoutSet.Broadcast(NewLayoutObject);
		}
	}
}

void UGameplayCommonUIPolicy::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);
	
	if (FGameplayPrimaryLayoutViewport* LayoutInfo = PrimaryViewportLayouts.FindByKey(LocalPlayer))
	{
		AddPrimaryLayoutToViewport(LocalPlayer, LayoutInfo->PrimaryLayout);
		LayoutInfo->bAddedToViewport = true;
	}
	else
	{
		CreatePrimaryLayout(LocalPlayer);
	}
}

void UGameplayCommonUIPolicy::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	if (FGameplayPrimaryLayoutViewport* LayoutInfo = PrimaryViewportLayouts.FindByKey(LocalPlayer))
	{
		RemovePrimaryLayoutFromViewport(LocalPlayer, LayoutInfo->PrimaryLayout);
		LayoutInfo->bAddedToViewport = false;

		UGameplayPrimaryLayout* RootLayout = LayoutInfo->PrimaryLayout;
		if (RootLayout && !RootLayout->IsDormant())
		{
			// We're removing a secondary player's root while it's in control - transfer control back to the primary player's root
			RootLayout->SetIsDormant(true);
			
			for (const FGameplayPrimaryLayoutViewport& RootLayoutInfo : PrimaryViewportLayouts)
			{
				if (RootLayoutInfo.LocalPlayer && RootLayoutInfo.LocalPlayer->IsPrimaryPlayer())
				{
					if (UGameplayPrimaryLayout* PrimaryRootLayout = RootLayoutInfo.PrimaryLayout)
					{
						PrimaryRootLayout->SetIsDormant(false);
					}
				}
			}
		}
	}
}

void UGameplayCommonUIPolicy::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
	NotifyPlayerRemoved(LocalPlayer);
	
	const int32 LayoutInfoIdx = PrimaryViewportLayouts.IndexOfByKey(LocalPlayer);
	if (LayoutInfoIdx != INDEX_NONE)
	{
		UGameplayPrimaryLayout* Layout = PrimaryViewportLayouts[LayoutInfoIdx].PrimaryLayout;
		PrimaryViewportLayouts.RemoveAt(LayoutInfoIdx);

		RemovePrimaryLayoutFromViewport(LocalPlayer, Layout);

		OnPrimaryLayoutReleased(LocalPlayer, Layout);
	}
}
