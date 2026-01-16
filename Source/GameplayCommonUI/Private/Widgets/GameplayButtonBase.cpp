// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayButtonBase.h"
#include "CommonActionWidget.h"
#include "CommonBorder.h"
#include "CommonLazyImage.h"
#include "CommonTextBlock.h"
#include "Subsystems/GameplayCommonUISubsystem.h"

UGameplayButtonBase::UGameplayButtonBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	bButtonTextUppercase = false;
}

void UGameplayButtonBase::NativePreConstruct()
{
	Super::NativePreConstruct();

	ReceiveUpdateButtonStyle();
	RefreshButtonText();
}

void UGameplayButtonBase::NativeConstruct()
{
	Super::NativeConstruct();
}

void UGameplayButtonBase::NativeOnHovered()
{
	Super::NativeOnHovered();

	if (!ButtonDescriptionText.IsEmpty())
	{
		if (UGameplayCommonUISubsystem* UIManager = UGameplayCommonUISubsystem::Get(this))
		{
			UIManager->NotifyButtonDescriptionTextChanged(this, ButtonDescriptionText);
		}
	}
}

void UGameplayButtonBase::NativeOnUnhovered()
{
	Super::NativeOnUnhovered();
	
	if (!ButtonDescriptionText.IsEmpty())
	{
		if (UGameplayCommonUISubsystem* UIManager = UGameplayCommonUISubsystem::Get(this))
		{
			UIManager->NotifyButtonDescriptionTextChanged(this, FText::GetEmpty());
		}
	}
}

void UGameplayButtonBase::OnInputMethodChanged(ECommonInputType CurrentInputType)
{
	Super::OnInputMethodChanged(CurrentInputType);

	ReceiveUpdateButtonStyle();
}

void UGameplayButtonBase::UpdateInputActionWidget()
{
	Super::UpdateInputActionWidget();

	ReceiveUpdateButtonStyle();
	RefreshButtonText();
}

void UGameplayButtonBase::NativeOnCurrentTextStyleChanged()
{
	Super::NativeOnCurrentTextStyleChanged();

	if (Text_Button && GetCurrentTextStyleClass())
	{
		Text_Button->SetStyle(GetCurrentTextStyleClass());
	}
}

void UGameplayButtonBase::SetButtonText(FText NewText)
{
	ButtonText = NewText;
	RefreshButtonText();
}

FText UGameplayButtonBase::GetButtonText() const
{
	if (Text_Button)
	{
		return Text_Button->GetText();
	}

	return FText::GetEmpty();
}

void UGameplayButtonBase::RefreshButtonText()
{
	if (ButtonText.IsEmpty())
	{
		if (InputActionWidget)
		{
			const FText ActionDisplayText = InputActionWidget->GetDisplayText();
			if (!ActionDisplayText.IsEmpty() && bShowInputDisplayTextIfPossible)
			{
				if (Text_Button)
				{
					Text_Button->SetText(ActionDisplayText);
					Text_Button->SetTextCase(bButtonTextUppercase);
				}
				
				ReceiveUpdateButtonText(ActionDisplayText);
				return;
			}
		}
	}

	if (Text_Button)
	{
		Text_Button->SetText(ButtonText);
		Text_Button->SetTextCase(bButtonTextUppercase);
	}
	
	ReceiveUpdateButtonText(ButtonText);
}

void UGameplayButtonBase::SetButtonBrush(const FSlateBrush& NewBrush)
{
	if (Image_Button)
	{
		Image_Button->SetBrush(NewBrush);
	}
}

void UGameplayButtonBase::SetButtonBrushFromLazyObject(TSoftObjectPtr<UObject> LazyObject)
{
	if (Image_Button)
	{
		Image_Button->SetBrushFromLazyDisplayAsset(LazyObject);
	}
}

FSlateBrush UGameplayButtonBase::GetButtonBrush() const
{
	if (Image_Button)
	{
		return Image_Button->GetBrush();
	}

	return FSlateBrush();
}
