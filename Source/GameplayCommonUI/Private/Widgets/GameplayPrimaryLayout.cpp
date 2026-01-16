// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayPrimaryLayout.h"
#include "Framework/GameplayCommonUIPolicy.h"
#include "Misc/GameplayCommonUILibrary.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystems/GameplayCommonUISubsystem.h"
#include "Widgets/CommonActivatableWidgetContainer.h"
#include "GameFramework/PlayerController.h"
#include "Engine/Engine.h"

DEFINE_LOG_CATEGORY(LogGameplayPrimaryLayout);

UGameplayPrimaryLayout::UGameplayPrimaryLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	
}

void UGameplayPrimaryLayout::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

UGameplayPrimaryLayout* UGameplayPrimaryLayout::GetPrimaryGameLayoutForPrimaryPlayer(const UObject* WorldContextObject)
{
	const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject);
	const APlayerController* PlayerController = GameInstance->GetPrimaryPlayerController(false);
	return GetPrimaryGameLayout(PlayerController);
}

UGameplayPrimaryLayout* UGameplayPrimaryLayout::GetPrimaryGameLayout(const APlayerController* PlayerController)
{
	return PlayerController ? GetPrimaryGameLayout(Cast<ULocalPlayer>(PlayerController->Player)) : nullptr;
}

UGameplayPrimaryLayout* UGameplayPrimaryLayout::GetPrimaryGameLayout(const ULocalPlayer* LocalPlayer)
{
	if (!LocalPlayer) return nullptr;
	
	const UGameInstance* GameInstance = LocalPlayer->GetGameInstance();
	if (!GameInstance)
	{
		return nullptr;
	}

	const UGameplayCommonUISubsystem* UIManager = GameInstance->GetSubsystem<UGameplayCommonUISubsystem>();
	if (!UIManager)
	{
		return nullptr;
	}

	const UGameplayCommonUIPolicy* Policy = UIManager->GetUIPolicy();
	if (!Policy)
	{
		return nullptr;
	}

	UGameplayPrimaryLayout* PrimaryLayout = Policy->GetPrimaryLayoutFromLocalPlayer(LocalPlayer);
	if (!PrimaryLayout)
	{
		return nullptr;
	}

	return PrimaryLayout;
}

void UGameplayPrimaryLayout::SetIsDormant(bool bInIsDormant)
{
	if (bIsDormant != bInIsDormant)
	{
		const ULocalPlayer* LP = GetOwningLocalPlayer();
		const int32 PlayerId = LP ? LP->GetControllerId() : -1;
		const TCHAR* OldDormancyStr = bIsDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* NewDormancyStr = bInIsDormant ? TEXT("Dormant") : TEXT("Not-Dormant");
		const TCHAR* PrimaryPlayerStr = LP && LP->IsPrimaryPlayer() ? TEXT("[Primary]") : TEXT("[Non-Primary]");
		UE_LOG(LogGameplayPrimaryLayout, Display, TEXT("%s GameplayPrimaryLayout Dormancy changed for [%d] from [%s] to [%s]"), PrimaryPlayerStr, PlayerId, OldDormancyStr, NewDormancyStr);

		bIsDormant = bInIsDormant;
		OnIsDormantChanged();
	}
}

void UGameplayPrimaryLayout::OnIsDormantChanged()
{
}

void UGameplayPrimaryLayout::FindAndRemoveWidgetFromLayer(UCommonActivatableWidget* ActivatableWidget)
{
	// We're not sure what layer the widget is on, so go searching.
	for (const auto& LayerKVP : RegisteredLayers)
	{
		LayerKVP.Value->RemoveWidget(*ActivatableWidget);
	}
}

void UGameplayPrimaryLayout::FindAndRemoveWidgetsFromLayer(FGameplayTag LayerTag)
{
	for (const auto& LayerKVP : RegisteredLayers)
	{
		if (LayerKVP.Key.MatchesTagExact(LayerTag))
		{
			LayerKVP.Value->ClearWidgets();
		}
	}
}

void UGameplayPrimaryLayout::ClearAllLayerStacks()
{
	for (const auto& KVP : RegisteredLayers)
	{
		KVP.Value->ClearWidgets();
	}
}

UCommonActivatableWidgetContainerBase* UGameplayPrimaryLayout::GetLayerStackContainer(FGameplayTag LayerTag)
{
	return RegisteredLayers.FindRef(LayerTag);
}

FString UGameplayPrimaryLayout::GetLayerNameFromContainer(const UCommonActivatableWidgetContainerBase* Container) const
{
	for (const auto& LayerKVP : RegisteredLayers)
	{
		if (LayerKVP.Value == Container)
		{
			return LayerKVP.Key.ToString();
		}
	}
	return TEXT("UnknownLayer");
}

void UGameplayPrimaryLayout::OnWidgetStackTransitioning(UCommonActivatableWidgetContainerBase* Widget, bool bIsTransitioning)
{
	if (bIsTransitioning)
	{
		const FName SuspendToken = UGameplayCommonUILibrary::SuspendInputForPlayer(GetOwningLocalPlayer(), TEXT("GlobalStackTransition"));
		SuspendInputTokens.Add(SuspendToken);
	}
	else
	{
		if (ensure(SuspendInputTokens.Num() > 0))
		{
			const FName SuspendToken = SuspendInputTokens.Pop();
			UGameplayCommonUILibrary::ResumeInputForPlayer(GetOwningLocalPlayer(), SuspendToken);
		}
	}
}

UCommonActivatableWidgetContainerBase* UGameplayPrimaryLayout::GetWidgetStackLayer(const FGameplayTag& LayerTag) const
{
	checkf(RegisteredLayers.Contains(LayerTag), TEXT("Can not find the widget stack by the tag %s"), *LayerTag.ToString());
	return RegisteredLayers.FindRef(LayerTag);
}

void UGameplayPrimaryLayout::RegisterLayerStack(FGameplayTag LayerTag, UCommonActivatableWidgetContainerBase* Stack)
{
	if (!IsDesignTime() && !RegisteredLayers.Contains(LayerTag))
	{
		Stack->OnTransitioningChanged.AddUObject(this, &UGameplayPrimaryLayout::OnWidgetStackTransitioning);
		
		// #todo: Helpe MI Epic Gaymes!
		// Stack->SetTransitionDuration(0.0);

		RegisteredLayers.Add(LayerTag, Stack);
		UE_LOG(LogGameplayPrimaryLayout, Log, TEXT("Layer Stack Registered under the tag %s"), *LayerTag.ToString());
	}
}
