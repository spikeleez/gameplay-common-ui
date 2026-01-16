// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingAction.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingAction)

UGameplaySettingAction::UGameplaySettingAction(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ActionText = FText::GetEmpty();
	NamedAction = FGameplayTag::EmptyTag;
	bDirtyAction = false;
}

void UGameplaySettingAction::OnInitialized()
{
	Super::OnInitialized();

#if !UE_BUILD_SHIPPING
	//ensureMsgf(HasCustomAction() || NamedAction.IsValid(), TEXT("Action settings need either a custom action or a named action."));
	ensureMsgf(!ActionText.IsEmpty(), TEXT("You must provide a ActionText for settings with actions."));
	ensureMsgf(!DescriptionRichText.IsEmpty(), TEXT("You must provide a description for settings with actions."));
#endif
}

void UGameplaySettingAction::SetCustomAction(TFunction<void(ULocalPlayer*)> InAction)
{
	CustomAction = UGameplayGameSettingCustomAction::CreateLambda([InAction](UGameplaySetting* InSetting, ULocalPlayer* InLocalPlayer)
	{
		InAction(InLocalPlayer);
	});
}

void UGameplaySettingAction::ExecuteAction()
{
	if (HasCustomAction())
	{
		CustomAction.ExecuteIfBound(this, LocalPlayer);
	}
	else
	{
		OnExecuteNamedActionEvent.Broadcast(this, NamedAction);
	}

	if (bDirtyAction)
	{
		NotifySettingChanged(EGameplaySettingChangeReason::Change);
	}
}