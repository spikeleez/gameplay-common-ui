// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplayCommonUISettings.h"
#include "Framework/GameplayCommonUIPolicy.h"
#include "UObject/ConstructorHelpers.h"
#include "Widgets/GameplayConfirmationDialog.h"

UGameplayCommonUISettings::UGameplayCommonUISettings(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	GameplayUIPolicyClass = UGameplayCommonUIPolicy::StaticClass();

	static ConstructorHelpers::FClassFinder<UGameplayConfirmationDialog> DefaultConfirmationDialogClass(TEXT("/GameplayCommonUI/Widgets/Confirmation/WBP_GameplayConfirmationDialog"));
	if (DefaultConfirmationDialogClass.Succeeded())
	{
		ConfirmationDialogClass = DefaultConfirmationDialogClass.Class;
	}

	static ConstructorHelpers::FClassFinder<UGameplayConfirmationDialog> DefaultErrorDialogClass(TEXT("/GameplayCommonUI/Widgets/Confirmation/WBP_GameplayErrorDialog"));
	if (DefaultErrorDialogClass.Succeeded())
	{
		ErrorDialogClass = DefaultErrorDialogClass.Class;
	}
}
