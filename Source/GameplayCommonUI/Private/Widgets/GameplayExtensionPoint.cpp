// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayExtensionPoint.h"
#include "GameFramework/PlayerState.h"
#include "Misc/UObjectToken.h"
#include "Subsystems/GameplayCommonLocalPlayerSubsystem.h"

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif

#define LOCTEXT_NAMESPACE "GameplayExtensionPointWidget"

UGameplayExtensionPoint::UGameplayExtensionPoint(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

#if WITH_EDITOR
void UGameplayExtensionPoint::ValidateCompiledDefaults(IWidgetCompilerLog& CompileLog) const
{
	Super::ValidateCompiledDefaults(CompileLog);

	// We don't care if the CDO doesn't have a specific tag.
	if (!HasAnyFlags(RF_ClassDefaultObject))
	{
		if (!ExtensionPointTag.IsValid())
		{
			const TSharedRef<FTokenizedMessage> Message = CompileLog.Error(FText::Format(LOCTEXT("UGameplayExtensionPointWidget", "{0} has no ExtensionPointTag specified - All extension points must specify a tag so they can be located."), FText::FromString(GetName())));
			Message->AddToken(FUObjectToken::Create(this));
		}
	}
}
#endif

void UGameplayExtensionPoint::ReleaseSlateResources(bool bReleaseChildren)
{
	ResetExtensionPoint();

	Super::ReleaseSlateResources(bReleaseChildren);
}

TSharedRef<SWidget> UGameplayExtensionPoint::RebuildWidget()
{
	if (!IsDesignTime() && ExtensionPointTag.IsValid())
	{
		ResetExtensionPoint();
		RegisterExtensionPoint();
		
		if (UGameplayCommonLocalPlayerSubsystem* UILocalPlayerSubsystem = GetOwningLocalPlayer()->GetSubsystem<UGameplayCommonLocalPlayerSubsystem>())
		{
			UILocalPlayerSubsystem->OnLocalPlayerStateSet.AddUObject(this, &ThisClass::RegisterExtensionPointForPlayerState);
		}
	}

	if (IsDesignTime())
	{
		auto GetExtensionPointText = [this]()
		{
			return FText::Format(LOCTEXT("DesignTime_ExtensionPointLabel", "Extension Point\n{0}"), FText::FromName(ExtensionPointTag.GetTagName()));
		};

		TSharedRef<SOverlay> MessageBox = SNew(SOverlay);

		MessageBox->AddSlot()
			.Padding(5.0f)
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Justification(ETextJustify::Center)
				.Text_Lambda(GetExtensionPointText)
			];

		return MessageBox;
	}
	
	return Super::RebuildWidget();
}

void UGameplayExtensionPoint::ResetExtensionPoint()
{
	ResetInternal();

	ExtensionMapping.Reset();
	for (FGameplayUIExtensionPointHandle& Handle : ExtensionPointHandles)
	{
		Handle.Unregister();
	}
	ExtensionPointHandles.Reset();
}

void UGameplayExtensionPoint::RegisterExtensionPoint()
{
	if (UGameplayCommonExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UGameplayCommonExtensionSubsystem>())
	{
		TArray<UClass*> AllowedDataClasses;
		AllowedDataClasses.Add(UUserWidget::StaticClass());
		AllowedDataClasses.Append(DataClasses);

		ExtensionPointHandles.Add(ExtensionSubsystem->RegisterExtensionPointInternal(
			ExtensionPointTag, 
			ExtensionPointTagMatch, 
			AllowedDataClasses, 
			FGameplayExtendUIExtensionPointSignature::CreateUObject(this, &ThisClass::OnAddOrRemoveExtension))
		);

		ExtensionPointHandles.Add(ExtensionSubsystem->RegisterExtensionPointForContextInternal(
			ExtensionPointTag, 
			GetOwningLocalPlayer(), 
			ExtensionPointTagMatch, 
			AllowedDataClasses,
			FGameplayExtendUIExtensionPointSignature::CreateUObject(this, &ThisClass::OnAddOrRemoveExtension))
		);
	}
}

void UGameplayExtensionPoint::RegisterExtensionPointForPlayerState(APlayerState* PlayerState)
{
	if (UGameplayCommonExtensionSubsystem* ExtensionSubsystem = GetWorld()->GetSubsystem<UGameplayCommonExtensionSubsystem>())
	{
		TArray<UClass*> AllowedDataClasses;
		AllowedDataClasses.Add(UUserWidget::StaticClass());
		AllowedDataClasses.Append(DataClasses);

		ExtensionPointHandles.Add(ExtensionSubsystem->RegisterExtensionPointForContextInternal(
			ExtensionPointTag, 
			PlayerState, 
			ExtensionPointTagMatch, 
			AllowedDataClasses,
			FGameplayExtendUIExtensionPointSignature::CreateUObject(this, &ThisClass::OnAddOrRemoveExtension))
		);
	}
}

void UGameplayExtensionPoint::OnAddOrRemoveExtension(EGameplayUIExtensionAction Action, const FGameplayUIExtensionRequest& Request)
{
	if (Action == EGameplayUIExtensionAction::Added)
	{
		UObject* Data = Request.Data;
		
		TSubclassOf<UUserWidget> WidgetClass(Cast<UClass>(Data));
		if (WidgetClass)
		{
			UUserWidget* Widget = CreateEntryInternal(WidgetClass);
			ExtensionMapping.Add(Request.ExtensionHandle, Widget);
		}
		else if (DataClasses.Num() > 0)
		{
			if (GetWidgetClassForData.IsBound())
			{
				WidgetClass = GetWidgetClassForData.Execute(Data);

				// If the data is irrelevant they can just return no widget class.
				if (WidgetClass)
				{
					if (UUserWidget* Widget = CreateEntryInternal(WidgetClass))
					{
						ExtensionMapping.Add(Request.ExtensionHandle, Widget);
						ConfigureWidgetForData.ExecuteIfBound(Widget, Data);
					}
				}
			}
		}
	}
	else
	{
		if (UUserWidget* Extension = ExtensionMapping.FindRef(Request.ExtensionHandle))
		{
			RemoveEntryInternal(Extension);
			ExtensionMapping.Remove(Request.ExtensionHandle);
		}
	}
}

#undef LOCTEXT_NAMESPACE
