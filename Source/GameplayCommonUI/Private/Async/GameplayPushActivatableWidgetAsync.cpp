// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Async/GameplayPushActivatableWidgetAsync.h"
#include "Framework/GameplayCommonUISettings.h"
#include "Subsystems/GameplayCommonUISubsystem.h"
#include "Engine/Engine.h"
#include "Engine/LocalPlayer.h"
#include "Widgets/GameplayPrimaryLayout.h"

DEFINE_LOG_CATEGORY_STATIC(LogGameplayPushActivatableWidgetAsync, Log, All);

UGameplayPushActivatableWidgetAsync::UGameplayPushActivatableWidgetAsync(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	MyLayerTag = FGameplayTag::EmptyTag;
	bSuspendInputUntilComplete = false;
}

UGameplayPushActivatableWidgetAsync* UGameplayPushActivatableWidgetAsync::PushActivatableWidgetClassToLayer(APlayerController* OwningPlayer, 
	TSoftClassPtr<UCommonActivatableWidget> WidgetClass, FGameplayTag LayerTag, bool bInSuspendInputUntilComplete)
{
	if (WidgetClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("PushSoftActivatableWidgetToLayer was passed a null WidgetClass"), ELogVerbosity::Error);
		return nullptr;
	}

	if (const UWorld* World = GEngine->GetWorldFromContextObject(OwningPlayer, EGetWorldErrorMode::LogAndReturnNull))
	{
		UGameplayPushActivatableWidgetAsync* Action = NewObject<UGameplayPushActivatableWidgetAsync>();
		Action->MyWidgetClass = WidgetClass;
		Action->OwningPlayerPtr = OwningPlayer;
		Action->MyLayerTag = LayerTag;
		Action->bSuspendInputUntilComplete = bInSuspendInputUntilComplete;
		Action->RegisterWithGameInstance(World);

		return Action;
	}

	return nullptr;
}

UGameplayPushActivatableWidgetAsync* UGameplayPushActivatableWidgetAsync::PushActivatableWidgetTagToLayer(APlayerController* OwningPlayer, 
	FGameplayTag WidgetTag, FGameplayTag LayerTag, bool bSuspendInputUntilComplete)
{
	if (!WidgetTag.IsValid())
	{
		FFrame::KismetExecutionMessage(TEXT("PushActivatableWidgetToLayer was passed a null WidgetTag"), ELogVerbosity::Error);
		return nullptr;
	}

	if (const UWorld* World = GEngine->GetWorldFromContextObject(OwningPlayer, EGetWorldErrorMode::LogAndReturnNull))
	{
		UGameplayPushActivatableWidgetAsync* Action = NewObject<UGameplayPushActivatableWidgetAsync>();
		Action->MyWidgetTag = WidgetTag;
		Action->OwningPlayerPtr = OwningPlayer;
		Action->MyLayerTag = LayerTag;
		Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
		Action->RegisterWithGameInstance(World);

		return Action;
	}

	return nullptr;
}

void UGameplayPushActivatableWidgetAsync::Activate()
{
	const UGameplayCommonUISettings* Settings = GetDefault<UGameplayCommonUISettings>();
	check(Settings);
	
	TSoftClassPtr<UCommonActivatableWidget> TargetWidgetClass = nullptr;
	if (!MyWidgetClass.IsNull())
	{
		TargetWidgetClass = MyWidgetClass;
	}
	else if (MyWidgetTag.IsValid())
	{
		TargetWidgetClass = Settings->RegisteredActivatableWidgets.FindRef(MyWidgetTag);
	}
	
	if (TargetWidgetClass.IsNull())
	{
		UE_LOG(LogGameplayPushActivatableWidgetAsync, Error, TEXT("GameplayPushActivatableWidgetAsync: not find any valid WidgetClass"));
		SetReadyToDestroy();
		return;
	}
	
	if (UGameplayPrimaryLayout* PrimaryLayout = UGameplayPrimaryLayout::GetPrimaryGameLayout(OwningPlayerPtr.Get()))
	{
		TWeakObjectPtr WeakThis = this;
		
		StreamingHandle = PrimaryLayout->PushWidgetToLayerStackAsync<UCommonActivatableWidget>(MyLayerTag, bSuspendInputUntilComplete, TargetWidgetClass,
		[this, WeakThis](EGameplayWidgetLayerAsyncState State, UCommonActivatableWidget* Widget)
		{
			if (!WeakThis.IsValid() || !IsValid(Widget)) return;

			switch (State)
			{
			case EGameplayWidgetLayerAsyncState::Initialize:
				BeforePush.Broadcast(Widget);
				break;
			case EGameplayWidgetLayerAsyncState::AfterPush:
				AfterPush.Broadcast(Widget);
				break;
			case EGameplayWidgetLayerAsyncState::Canceled:
				break;
			}

			SetReadyToDestroy();
		});
	}
	else
	{
		// If the primary layout is not yet valid, we wait for it to be set.
		if (const APlayerController* PlayerController = OwningPlayerPtr.Get())
		{
			if (UGameplayCommonUISubsystem* UIManager = UGameplayCommonUISubsystem::Get(PlayerController))
			{
				UIManager->OnPrimaryLayoutSet.AddUObject(this, &ThisClass::OnPrimaryLayoutSet);
				return;
			}
		}

		UE_LOG(LogGameplayPushActivatableWidgetAsync, Error, TEXT("GameplayPushActivatableWidgetAsync: PrimaryLayout is not valid and could not find UIManager to wait."));
		SetReadyToDestroy();
	}
}

void UGameplayPushActivatableWidgetAsync::OnPrimaryLayoutSet(UGameplayPrimaryLayout* PrimaryLayout)
{
	if (const APlayerController* PlayerController = OwningPlayerPtr.Get())
	{
		if (UGameplayCommonUISubsystem* UIManager = UGameplayCommonUISubsystem::Get(PlayerController))
		{
			UIManager->OnPrimaryLayoutSet.RemoveAll(this);
		}
	}
	
	Activate();
}

void UGameplayPushActivatableWidgetAsync::Cancel()
{
	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
	
	Super::Cancel();
}
