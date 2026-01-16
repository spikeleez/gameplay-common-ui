// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayActivatableWidget.h"
#include "Input/CommonUIInputTypes.h"
#include "Misc/GameplayCommonTypes.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif

#define LOCTEXT_NAMESPACE "GameplayActivatableWidget"

DEFINE_LOG_CATEGORY_STATIC(LogGameplayActivatableWidget, Log, All);

UGameplayActivatableWidget::UGameplayActivatableWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetIsFocusable(true);
	
	InputMode = EGameplayActivatableInputMode::Default;
	MouseCaptureMode = EMouseCaptureMode::CapturePermanently;
	bHidesCursorDuringViewportCapture = true;
}

void UGameplayActivatableWidget::NativeDestruct()
{
	UnregisterAllBoundActionBindings();
		
	Super::NativeDestruct();
}

void UGameplayActivatableWidget::NativeOnActivated()
{
	Super::NativeOnActivated();
	
	UE_LOG(LogGameplayActivatableWidget, Display, TEXT("[Activation] Widget '%s' activated."), *GetName());
	UE_LOG(LogGameplayActivatableWidget, Verbose, TEXT("  > Input Mode: %d | Mouse Capture: %d"), (int32)InputMode, (int32)MouseCaptureMode);
}

void UGameplayActivatableWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	
	UE_LOG(LogGameplayActivatableWidget, Display, TEXT("[Deactivation] Widget '%s' deactivated."), *GetName());
}

TOptional<FUIInputConfig> UGameplayActivatableWidget::GetDesiredInputConfig() const
{
	UE_LOG(LogGameplayActivatableWidget, VeryVerbose, TEXT("Widget '%s' requesting InputConfig for mode %d"), *GetName(), (int32)InputMode);
	
	switch (InputMode)
	{
	case EGameplayActivatableInputMode::Menu:
		return FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, false);
	case EGameplayActivatableInputMode::Game:
		return FUIInputConfig(ECommonInputMode::Game, MouseCaptureMode, bHidesCursorDuringViewportCapture);
	case EGameplayActivatableInputMode::GameMenu:
		return FUIInputConfig(ECommonInputMode::All, MouseCaptureMode, bHidesCursorDuringViewportCapture);
	case EGameplayActivatableInputMode::Default:
	default:
		return TOptional<FUIInputConfig>();
	}
}

#if WITH_EDITOR
void UGameplayActivatableWidget::ValidateCompiledWidgetTree(const UWidgetTree& BlueprintWidgetTree, IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledWidgetTree(BlueprintWidgetTree, CompileLog);
	
	if (!GetClass()->IsFunctionImplementedInScript(GET_FUNCTION_NAME_CHECKED(UGameplayActivatableWidget, BP_GetDesiredFocusTarget)))
	{
		if (GetParentNativeClass(GetClass()) == UGameplayActivatableWidget::StaticClass())
		{
			CompileLog.Warning(LOCTEXT("ValidateGetDesiredFocusTarget_Warning", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen."));
		}
		else
		{
			CompileLog.Note(LOCTEXT("ValidateGetDesiredFocusTarget_Note", "GetDesiredFocusTarget wasn't implemented, you're going to have trouble using gamepads on this screen. If it was implemented in the native base class you can ignore this message."));
		}
	}
}
#endif

void UGameplayActivatableWidget::RegisterBoundActionBinding(FDataTableRowHandle InputAction, const FOnInputActionExecutedSignature& Callback, FUIActionBindingHandle& BindingHandle)
{
	FBindUIActionArgs BindArgs(InputAction, FSimpleDelegate::CreateLambda([InputAction, Callback]()
	{
		Callback.ExecuteIfBound(InputAction.RowName);
	}));
	BindArgs.bDisplayInActionBar = true;

	BindingHandle = RegisterUIActionBinding(BindArgs);
	BindingHandles.Add(BindingHandle);
}

void UGameplayActivatableWidget::RegisterBoundEnhancedActionBinding(UInputAction* InputAction, const FOnEnhancedInputActionExecutedSignature& Callback, FUIActionBindingHandle& BindingHandle)
{
	FBindUIActionArgs BindArgs(InputAction, FSimpleDelegate::CreateLambda([InputAction, Callback]()
	{
		Callback.ExecuteIfBound(InputAction);
	}));
	BindArgs.bDisplayInActionBar = true;

	BindingHandle = RegisterUIActionBinding(BindArgs);
	BindingHandles.Add(BindingHandle);
}

void UGameplayActivatableWidget::UnregisterBoundActionBinding(FUIActionBindingHandle& BindingHandle)
{
	if (BindingHandle.IsValid())
	{
		BindingHandle.Unregister();
		BindingHandles.Remove(BindingHandle);
	}
}

void UGameplayActivatableWidget::UnregisterAllBoundActionBindings()
{
	if (BindingHandles.IsEmpty()) return;
	
	for (FUIActionBindingHandle Handle : BindingHandles)
	{
		if (Handle.IsValid())
		{
			Handle.Unregister();
		}
	}
	
	BindingHandles.Empty();
}

#undef LOCTEXT_NAMESPACE