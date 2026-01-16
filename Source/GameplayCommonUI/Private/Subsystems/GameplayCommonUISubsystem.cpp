// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Subsystems/GameplayCommonUISubsystem.h"
#include "CanvasItem.h"
#include "DisplayDebugHelpers.h"
#include "Framework/GameplayCommonUISettings.h"
#include "Framework/GameplayCommonUIPolicy.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Engine/Engine.h" // For GEngine

UGameplayCommonUISubsystem* UGameplayCommonUISubsystem::Get(const UObject* WorldContextObject)
{
	if (!WorldContextObject) return nullptr;
	
	if (const UGameInstance* GameInstance = WorldContextObject->GetWorld()->GetGameInstance())
	{
		return GameInstance->GetSubsystem<UGameplayCommonUISubsystem>();
	}
	
	return nullptr;
}

bool UGameplayCommonUISubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!CastChecked<UGameInstance>(Outer)->IsDedicatedServerInstance())
	{
		TArray<UClass*> ChildClasses;
		GetDerivedClasses(GetClass(), ChildClasses, false);

		// Only create an instance if there is no override implementation defined elsewhere
		return ChildClasses.Num() == 0;
	}

	return false;
}

void UGameplayCommonUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (!CurrentPolicy)
	{
		const UGameplayCommonUISettings* Settings = GetDefault<UGameplayCommonUISettings>();
		check(Settings);

		const TSubclassOf<UGameplayCommonUIPolicy> PolicyClass = Settings->GameplayUIPolicyClass.LoadSynchronous();
		if (ensure(PolicyClass))
		{
			SwitchToPolicy(NewObject<UGameplayCommonUIPolicy>(this, PolicyClass));
		}
	}
	
	if (!IsTemplate())
	{
		AHUD::OnShowDebugInfo.AddUObject(this, &ThisClass::OnShowDebugInfo);
	}
}

void UGameplayCommonUISubsystem::Deinitialize()
{
	AHUD::OnShowDebugInfo.RemoveAll(this);
	
	SwitchToPolicy(nullptr);
	
	Super::Deinitialize();
}

void UGameplayCommonUISubsystem::OnShowDebugInfo(AHUD* HUD, UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& YL, float& YPos)
{
	static const FName Name_CommonUI(TEXT("GameplayCommonUI"));
	if (!Canvas || !HUD || !DisplayInfo.IsDisplayOn(Name_CommonUI))
	{
		return;
	}
	
	const UFont* RenderFont = GEngine->GetSmallFont();
	constexpr float LineHeight = 15.0f;
	YL = LineHeight;
	
	constexpr FLinearColor ColorHeader(0.1f, 0.7f, 0.1f);
	constexpr FLinearColor ColorLayer(0.2f, 0.5f, 1.0f);
	constexpr FLinearColor ColorActive(1.0f, 0.8f, 0.2f);
	constexpr FLinearColor ColorInactive(0.5f, 0.5f, 0.5f);
	
	const UGameplayPrimaryLayout* PrimaryLayout = CurrentPolicy ? CurrentPolicy->GetPrimaryLayout() : nullptr;
	
	auto DrawRow = [&](const FString& StatusText, const FString& NameText, const FLinearColor& Color, float Indent)
	{
		FCanvasTextItem StatusItem(FVector2D(10.0f + Indent, YPos), FText::FromString(StatusText), RenderFont, Color);
		StatusItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(StatusItem);

		FCanvasTextItem NameItem(FVector2D(10.0f + Indent + 85.0f, YPos), FText::FromString(NameText), RenderFont, Color);
		NameItem.EnableShadow(FLinearColor::Black);
		Canvas->DrawItem(NameItem);
		
		YPos += LineHeight;
	};
	
	// HEADER
	YPos += 5.0f;
	const FString HeaderText = FString::Printf(TEXT("GAMEPLAY COMMON UI DEBUGGER  [State: %s]"), PrimaryLayout ? (PrimaryLayout->IsDormant() ? TEXT("Dormant") : TEXT("Active")) : TEXT("Null"));
	
	FCanvasTextItem HeaderItem(FVector2D(5.0f, YPos), FText::FromString(HeaderText), RenderFont, ColorHeader);
	HeaderItem.EnableShadow(FLinearColor::Black);
	HeaderItem.Scale = FVector2D(1.1f, 1.1f);
	Canvas->DrawItem(HeaderItem);
	YPos += LineHeight + 5.0f;
	
	if (!PrimaryLayout)
	{
		return;
	}
	
	const TMap<FGameplayTag, UCommonActivatableWidgetContainerBase*>& Layers = PrimaryLayout->GetRegisteredLayers();
	for (const auto& LayerKVP : Layers)
	{
		const FGameplayTag& LayerTag = LayerKVP.Key;
		const UCommonActivatableWidgetContainerBase* Container = LayerKVP.Value;
		if (!Container) continue;

		FString LayerName = LayerTag.GetTagName().ToString();
		DrawRow(TEXT("Layer:"), LayerName, ColorLayer, 0.0f);

		const TArray<UCommonActivatableWidget*>& WidgetList = Container->GetWidgetList();
		const UCommonActivatableWidget* ActiveWidget = Container->GetActiveWidget();
		
		if (WidgetList.Num() > 0)
		{
			for (int32 i = WidgetList.Num() - 1; i >= 0; --i)
			{
				const UCommonActivatableWidget* Widget = WidgetList[i];
				if (!Widget) continue;

				const bool bIsActive = (Widget == ActiveWidget);
                
				FString StatusStr = bIsActive ? TEXT("[Active]") : TEXT("[Inactive]");
				FLinearColor RowColor = bIsActive ? ColorActive : ColorInactive;
				
				FString WidgetName = FString::Printf(TEXT("[%d] %s"), i, *Widget->GetName());

				DrawRow(StatusStr, WidgetName, RowColor, 20.0f);
			}
		}
		else
		{
			DrawRow(TEXT("[Empty]"), TEXT("-"), ColorInactive, 20.0f);
		}

		YPos += 8.0f;
	}
}

void UGameplayCommonUISubsystem::NotifyPlayerAdded(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerAdded(LocalPlayer);
	}
}

void UGameplayCommonUISubsystem::NotifyPlayerRemoved(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerRemoved(LocalPlayer);
	}
}

void UGameplayCommonUISubsystem::NotifyPlayerDestroyed(ULocalPlayer* LocalPlayer)
{
	if (ensure(LocalPlayer) && CurrentPolicy)
	{
		CurrentPolicy->NotifyPlayerDestroyed(LocalPlayer);
	}
}

void UGameplayCommonUISubsystem::NotifyButtonDescriptionTextChanged(UGameplayButtonBase* Button, const FText& NewDescriptionText)
{
	if (OnButtonDescriptionChanged.IsBound())
	{
		OnButtonDescriptionChanged.Broadcast(Button, NewDescriptionText);
	}
}

void UGameplayCommonUISubsystem::SwitchToPolicy(UGameplayCommonUIPolicy* NewPolicy)
{
	if (CurrentPolicy != NewPolicy)
	{
		CurrentPolicy = NewPolicy;
	}
}