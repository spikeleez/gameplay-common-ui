// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Widgets/GameplayTabListWidgetBase.h"
#include "CommonAnimatedSwitcher.h"
#include "CommonButtonBase.h"

UGameplayTabListWidgetBase::UGameplayTabListWidgetBase(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UGameplayTabListWidgetBase::NativeOnInitialized()
{
	Super::NativeOnInitialized();
}

void UGameplayTabListWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	SetupTabs();
}

void UGameplayTabListWidgetBase::NativeDestruct()
{
	for (FGameplayTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		if (TabInfo.CreatedTabContentWidget)
		{
			TabInfo.CreatedTabContentWidget->RemoveFromParent();
			TabInfo.CreatedTabContentWidget = nullptr;
		}
	}
	
	Super::NativeDestruct();
}

bool UGameplayTabListWidgetBase::GetPreregisteredTabDescriptor(const FName TabNameId, FGameplayTabDescriptor& OutTabInfo)
{
	const FGameplayTabDescriptor* const FoundTabInfo = PreregisteredTabInfoArray.FindByPredicate([&](const FGameplayTabDescriptor& TabInfo)
	{
		return TabInfo.TabId == TabNameId;
	});

	if (!FoundTabInfo)
	{
		return false;
	}

	OutTabInfo = *FoundTabInfo;
	return true;
}

bool UGameplayTabListWidgetBase::GetTabDescriptor(const FName TabNameId, FGameplayTabDescriptor& OutTabInfo)
{
	if (const FGameplayTabDescriptor* FoundDynamicTab = RegisteredTabDescriptors.Find(TabNameId))
	{
		OutTabInfo = *FoundDynamicTab;
		return true;
	}
	
	const FGameplayTabDescriptor* const FoundPreregisteredTabInfo = PreregisteredTabInfoArray.FindByPredicate([&](const FGameplayTabDescriptor& TabInfo)
	{
		return TabInfo.TabId == TabNameId;
	});

	if (FoundPreregisteredTabInfo)
	{
		OutTabInfo = *FoundPreregisteredTabInfo;
		return true;
	}

	return false;
}

void UGameplayTabListWidgetBase::SetTabHiddenState(FName TabNameId, bool bHidden)
{
	for (FGameplayTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		if (TabInfo.TabId == TabNameId)
		{
			TabInfo.bHidden = bHidden;
			break;
		}
	}
}

bool UGameplayTabListWidgetBase::RegisterDynamicTab(const FGameplayTabDescriptor& TabDescriptor)
{
	// If it's hidden we just ignore it.
	if (TabDescriptor.bHidden)
	{
		return true;
	}
	
	FGameplayTabDescriptor NewTabDescritor = TabDescriptor;
	RegisteredTabDescriptors.Add(NewTabDescritor.TabId, NewTabDescritor);
	
	if (NewTabDescritor.CreatedTabContentWidget == nullptr && NewTabDescritor.TabContentType != nullptr)
	{
		NewTabDescritor.CreatedTabContentWidget = CreateWidget<UCommonUserWidget>(GetOwningPlayer(), NewTabDescritor.TabContentType);
		if (NewTabDescritor.CreatedTabContentWidget)
		{
			OnTabContentCreatedNative.Broadcast(NewTabDescritor.TabId, Cast<UCommonUserWidget>(NewTabDescritor.CreatedTabContentWidget));
			OnTabContentCreated.Broadcast(NewTabDescritor.TabId, Cast<UCommonUserWidget>(NewTabDescritor.CreatedTabContentWidget));
		}
	}
	
	if (NewTabDescritor.CreatedTabContentWidget)
	{
		if (UCommonAnimatedSwitcher* CurrentLinkedSwitcher = GetLinkedSwitcher())
		{
			if (!CurrentLinkedSwitcher->HasChild(NewTabDescritor.CreatedTabContentWidget))
			{
				CurrentLinkedSwitcher->AddChild(NewTabDescritor.CreatedTabContentWidget);
			}
		}
	}
	
	PendingTabLabelInfoMap.Add(NewTabDescritor.TabId, NewTabDescritor);
	
	return RegisterTab(NewTabDescritor.TabId, NewTabDescritor.TabButtonType, NewTabDescritor.CreatedTabContentWidget);
}

void UGameplayTabListWidgetBase::HandlePreLinkedSwitcherChanged()
{
	for (const FGameplayTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		// Remove tab content widget from linked switcher, as it is being disassociated.
		if (TabInfo.CreatedTabContentWidget)
		{
			TabInfo.CreatedTabContentWidget->RemoveFromParent();
		}
	}

	Super::HandlePreLinkedSwitcherChanged();
}

void UGameplayTabListWidgetBase::HandlePostLinkedSwitcherChanged()
{
	if (!IsDesignTime() && GetCachedWidget().IsValid())
	{
		// Don't bother making tabs if we're in the designer or haven't been constructed yet
		SetupTabs();
	}

	Super::HandlePostLinkedSwitcherChanged();
}

void UGameplayTabListWidgetBase::HandleTabCreation_Implementation(FName TabId, UCommonButtonBase* TabButton)
{
	FGameplayTabDescriptor* TabInfoPtr;
	
	FGameplayTabDescriptor TabInfo;
	if (GetPreregisteredTabDescriptor(TabId, TabInfo))
	{
		TabInfoPtr = &TabInfo;
	}
	else
	{
		TabInfoPtr = PendingTabLabelInfoMap.Find(TabId);
	}
	
	if (TabButton->GetClass()->ImplementsInterface(UGameplayTabButtonInterface::StaticClass()))
	{
		if (ensureMsgf(TabInfoPtr, TEXT("A tab button was created with id %s but no label info was specified. RegisterDynamicTab should be used over RegisterTab to provide label info."),
			*TabId.ToString()))
		{
			IGameplayTabButtonInterface::Execute_SetTabLabelInfo(TabButton, *TabInfoPtr);
		}
	}

	PendingTabLabelInfoMap.Remove(TabId);
}

bool UGameplayTabListWidgetBase::IsFirstTabActive() const
{
	if (PreregisteredTabInfoArray.Num() > 0)
	{
		return GetActiveTab() == PreregisteredTabInfoArray[0].TabId;
	}

	return false;
}

bool UGameplayTabListWidgetBase::IsLastTabActive() const
{
	if (PreregisteredTabInfoArray.Num() > 0)
	{
		return GetActiveTab() == PreregisteredTabInfoArray.Last().TabId;
	}

	return false;
}

bool UGameplayTabListWidgetBase::IsTabVisible(FName TabId)
{
	if (const UCommonButtonBase* Button = GetTabButtonBaseByID(TabId))
	{
		const ESlateVisibility TabVisibility = Button->GetVisibility();
		return (TabVisibility == ESlateVisibility::Visible
			|| TabVisibility == ESlateVisibility::HitTestInvisible
			|| TabVisibility == ESlateVisibility::SelfHitTestInvisible);
	}

	return false;
}

int32 UGameplayTabListWidgetBase::GetVisibleTabCount()
{
	int32 Result = 0;
	const int32 TabCount = GetTabCount();
	for ( int32 Index = 0; Index < TabCount; Index++ )
	{
		if (IsTabVisible(GetTabIdAtIndex( Index )))
		{
			Result++;
		}
	}

	return Result;
}

void UGameplayTabListWidgetBase::SetupTabs()
{
	for (FGameplayTabDescriptor& TabInfo : PreregisteredTabInfoArray)
	{
		if (TabInfo.bHidden)
		{
			continue;
		}

		// If the tab content hasn't been created already, create it.
		if (!TabInfo.CreatedTabContentWidget && TabInfo.TabContentType)
		{
			TabInfo.CreatedTabContentWidget = CreateWidget<UCommonUserWidget>(GetOwningPlayer(), TabInfo.TabContentType);
			OnTabContentCreatedNative.Broadcast(TabInfo.TabId, Cast<UCommonUserWidget>(TabInfo.CreatedTabContentWidget));
			OnTabContentCreated.Broadcast(TabInfo.TabId, Cast<UCommonUserWidget>(TabInfo.CreatedTabContentWidget));
		}

		if (UCommonAnimatedSwitcher* CurrentLinkedSwitcher = GetLinkedSwitcher())
		{
			// Add the tab content to the newly linked switcher.
			if (!CurrentLinkedSwitcher->HasChild(TabInfo.CreatedTabContentWidget))
			{
				CurrentLinkedSwitcher->AddChild(TabInfo.CreatedTabContentWidget);
			}
		}

		// If the tab is not already registered, register it.
		if (GetTabButtonBaseByID(TabInfo.TabId) == nullptr)
		{
			RegisterTab(TabInfo.TabId, TabInfo.TabButtonType, TabInfo.CreatedTabContentWidget);
		}
	}
}
