// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplaySettingPressAnyKey.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingPressAnyKey)

class ICursor;

class FGameplayPressAnyKeyInputPreProcessor final : public IInputProcessor
{
public:
	FGameplayPressAnyKeyInputPreProcessor()
	{}

	virtual void Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor) override
	{}

	virtual bool HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		HandleKey(InKeyEvent.GetKey());
		return true;
	}

	virtual bool HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent) override
	{
		return true;
	}

	virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		HandleKey(MouseEvent.GetEffectingButton());
		return true;
	}

	virtual bool HandleMouseButtonDownEvent(FSlateApplication& SlateApp, const FPointerEvent& MouseEvent) override
	{
		return true;
	}

	virtual bool HandleMouseWheelOrGestureEvent(FSlateApplication& SlateApp, const FPointerEvent& InWheelEvent, const FPointerEvent* InGestureEvent) override
	{
		if (InWheelEvent.GetWheelDelta() != 0.0f)
		{
			const FKey Key = InWheelEvent.GetWheelDelta() < 0.0f ? EKeys::MouseScrollDown : EKeys::MouseScrollUp;
			HandleKey(Key);
		}
		
		return true;
	}

	DECLARE_MULTICAST_DELEGATE(FGameplayPressAnyKeyInputPreProcessorCancelled);
	FGameplayPressAnyKeyInputPreProcessorCancelled OnKeySelectionCancelled;

	DECLARE_MULTICAST_DELEGATE_OneParam(FGameplayPressAnyKeyInputPreProcessorKeySelected, FKey);
	FGameplayPressAnyKeyInputPreProcessorKeySelected OnKeySelected;

private:
	void HandleKey(const FKey& InKey)
	{
		// Cancel this process if it's Escape, Touch, or Gamepad key.
		if (InKey == EKeys::LeftCommand || InKey == EKeys::RightCommand)
		{
			// Ignore.
		}
		else if (InKey == EKeys::Escape || InKey.IsTouch() || InKey.IsGamepadKey())
		{
			OnKeySelectionCancelled.Broadcast();
		}
		else
		{
			OnKeySelected.Broadcast(InKey);
		}
	}
};

UGameplaySettingPressAnyKey::UGameplaySettingPressAnyKey(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameplaySettingPressAnyKey::NativeOnActivated()
{
	Super::NativeOnActivated();

	bKeySelected = false;

	InputProcessor = MakeShared<FGameplayPressAnyKeyInputPreProcessor>();
	InputProcessor->OnKeySelected.AddUObject(this, &UGameplaySettingPressAnyKey::HandleKeySelected);
	InputProcessor->OnKeySelectionCancelled.AddUObject(this, &UGameplaySettingPressAnyKey::HandleKeySelectionCancelled);
	FSlateApplication::Get().RegisterInputPreProcessor(InputProcessor, 0);
}

void UGameplaySettingPressAnyKey::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);
	}
}

void UGameplaySettingPressAnyKey::HandleKeySelected(FKey InKey)
{
	if (!bKeySelected)
	{
		bKeySelected = true;
		Dismiss([this, InKey]()
		{
			OnKeySelected.Broadcast(InKey);
		});
	}
}

void UGameplaySettingPressAnyKey::HandleKeySelectionCancelled()
{
	if (!bKeySelected)
	{
		bKeySelected = true;
		Dismiss([this]()
		{
			OnKeySelectionCanceled.Broadcast();
		});
	}
}

void UGameplaySettingPressAnyKey::Dismiss(TFunction<void()> PostDismissCallback)
{
	// We delay a tick so that we're done processing input.
	FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateWeakLambda(this, [this, PostDismissCallback](float DeltaTime)
	{
		QUICK_SCOPE_CYCLE_COUNTER(STAT_UGameplayGameSettingPressAnyKey_Dismiss);

		FSlateApplication::Get().UnregisterInputPreProcessor(InputProcessor);

		DeactivateWidget();

		PostDismissCallback();

		return false;
	}));
}
