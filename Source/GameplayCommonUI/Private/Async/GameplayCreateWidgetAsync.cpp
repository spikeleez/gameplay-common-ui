// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Async/GameplayCreateWidgetAsync.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/Engine.h"
#include "Engine/AssetManager.h"
#include "Engine/GameInstance.h"
#include "Misc/GameplayCommonUILibrary.h"

static const FName InputFilterReason_Template = FName(TEXT("CreatingWidgetAsync"));

UGameplayCreateWidgetAsync::UGameplayCreateWidgetAsync(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, bSuspendInputUntilComplete(true)
{
}

UGameplayCreateWidgetAsync* UGameplayCreateWidgetAsync::CreateWidgetAsync(UObject* WorldContextObject, APlayerController* OwningPlayer, 
	TSoftClassPtr<UUserWidget> WidgetClass, bool bSuspendInputUntilComplete)
{
	if (WidgetClass.IsNull())
	{
		FFrame::KismetExecutionMessage(TEXT("CreateWidgetAsync was passed a null WidgetClass"), ELogVerbosity::Error);
		return nullptr;
	}

	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);

	UGameplayCreateWidgetAsync* Action = NewObject<UGameplayCreateWidgetAsync>();
	Action->UserWidgetSoftClass = WidgetClass;
	Action->OwningPlayer = OwningPlayer;
	Action->World = World;
	Action->GameInstance = World->GetGameInstance();
	Action->bSuspendInputUntilComplete = bSuspendInputUntilComplete;
	Action->RegisterWithGameInstance(World);

	return Action;
}

void UGameplayCreateWidgetAsync::Activate()
{
	SuspendInputToken = bSuspendInputUntilComplete ? UGameplayCommonUILibrary::SuspendInputForPlayer(OwningPlayer.Get(), InputFilterReason_Template) : NAME_None;

	StreamingHandle = UAssetManager::Get().GetStreamableManager().RequestAsyncLoad(UserWidgetSoftClass.ToSoftObjectPath(), 
		FStreamableDelegate::CreateUObject(this, &ThisClass::OnWidgetLoaded), FStreamableManager::AsyncLoadHighPriority);

	// Set up a cancel delegate so that we can resume input if this handler is canceled.
	StreamingHandle->BindCancelDelegate(FStreamableDelegate::CreateWeakLambda(this, [this]()
	{
		UGameplayCommonUILibrary::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
	}));
}

void UGameplayCreateWidgetAsync::Cancel()
{
	if (StreamingHandle.IsValid())
	{
		StreamingHandle->CancelHandle();
		StreamingHandle.Reset();
	}
	
	Super::Cancel();
}

void UGameplayCreateWidgetAsync::OnWidgetLoaded()
{
	if (bSuspendInputUntilComplete)
	{
		UGameplayCommonUILibrary::ResumeInputForPlayer(OwningPlayer.Get(), SuspendInputToken);
	}

	// If the load as successful, create it, otherwise don't complete this.
	const TSubclassOf<UUserWidget> UserWidgetClass = UserWidgetSoftClass.Get();
	if (UserWidgetClass)
	{
		UUserWidget* UserWidget = UWidgetBlueprintLibrary::Create(World.Get(), UserWidgetClass, OwningPlayer.Get());
		OnComplete.Broadcast(UserWidget);
	}

	StreamingHandle.Reset();
	SetReadyToDestroy();
}
