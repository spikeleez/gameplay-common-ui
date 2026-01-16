// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Styling/SlateBrush.h"
#include "CommonButtonBase.h"
#include "CommonUserWidget.h"
#include "GameplayCommonTypes.generated.h"

class UGameplayPrimaryLayout;
class UWidget;
class ULocalPlayer;

/** @brief Defines the available input modes for activatable widgets */
UENUM(BlueprintType)
enum class EGameplayActivatableInputMode : uint8
{
	/** Default behavior, inherited from parent or system defaults */
	Default						UMETA(DisplayName = "Default"),

	/** Menu-only input mode (UI focus) */
	Menu						UMETA(DisplayName = "Menu"),

	/** Game-only input mode (Player control focus) */
	Game						UMETA(DisplayName = "Game"),

	/** Simultaneous game and menu input mode */
	GameMenu					UMETA(DisplayName = "Game & Menu")
};

/** @brief Results representing the outcome of a confirmation dialog */
UENUM(BlueprintType)
enum class EGameplayConfirmationResult : uint8
{
	/** The "yes" or positive button was pressed */
	Confirmed					UMETA(DisplayName = "Confirmed"),
	
	/** The "no" or negative button was pressed */
	Declined					UMETA(DisplayName = "Declined"),

	/** The "ignore" or "cancel" button was pressed */
	Cancelled					UMETA(DisplayName = "Cancelled"),

	/** The dialog was closed or killed without specific user input */
	Killed						UMETA(DisplayName = "Killed"),
	
	/** Unknown or uninitialized status */
	Unknown						UMETA(Hidden)
};

/** @brief State representing the progress of an asynchronous widget push operation */
UENUM(BlueprintType)
enum class EGameplayPushWidgetAsyncState : uint8
{
	/** Load is starting, before the widget is actually pushed to the layer */
	BeforePush,

	/** Load is complete and the widget has been pushed */
	AfterPush
};

/** @brief State representing the progress of a widget layer's asynchronous lifecycle */
UENUM(BlueprintType)
enum class EGameplayWidgetLayerAsyncState : uint8
{
	/** Operation was canceled */
	Canceled,

	/** Operation is starting initialization */
	Initialize,
	
	/** Operation finished pushing to the layer */
	AfterPush
};

/** @brief Represents a single actionable button within a confirmation dialog */
USTRUCT(BlueprintType)
struct FGameplayConfirmationDialogAction
{
	GENERATED_BODY()

	/** The result that will be broadcast if this button is clicked */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialog")
	EGameplayConfirmationResult DialogResult;
	
	/** The localized text displayed on the button */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Dialog")
	FText ButtonDisplayText;

	FGameplayConfirmationDialogAction()
		: DialogResult(EGameplayConfirmationResult::Unknown)
		, ButtonDisplayText(FText::GetEmpty())
	{}
	FGameplayConfirmationDialogAction(EGameplayConfirmationResult InDialogResult, const FText& InButtonDisplayText)
		: DialogResult(InDialogResult)
		, ButtonDisplayText(InButtonDisplayText)
	{}

	bool operator==(const FGameplayConfirmationDialogAction& Other) const
	{
		return DialogResult == Other.DialogResult && ButtonDisplayText.EqualTo(Other.ButtonDisplayText);
	}
};

/** @brief Configuration data for individual tabs within a tab list widget */
USTRUCT(BlueprintType)
struct FGameplayTabDescriptor
{
	GENERATED_BODY()
	
	FGameplayTabDescriptor()
		: TabId(NAME_None)
		, TabAssetTags(FGameplayTagContainer::EmptyContainer)
		, TabDisplayName(FText::GetEmpty())
		, bHidden(false)
	{}
	
	bool operator==(const FGameplayTabDescriptor& Other) const
	{
		return TabId == Other.TabId && TabAssetTags == Other.TabAssetTags && TabDisplayName.EqualTo(Other.TabDisplayName);
	}

public:
	/** Internal unique identifier for the tab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tab")
	FName TabId;
	
	/** Metadata tags associated with the tab (useful for filtering or appearance mapping) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tab")
	FGameplayTagContainer TabAssetTags;

	/** Localized name shown on the tab button */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tab")
	FText TabDisplayName;

	/** Visual icon for the tab */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tab")
	FSlateBrush TabIcon;

	/** Whether this tab is currently hidden from the user */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tab")
	uint8 bHidden : 1;

	/** The class of the button to use for this tab in the tab list */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tab")
	TSubclassOf<UCommonButtonBase> TabButtonType;

	/** The class of the content widget to display when this tab is active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Tab")
	TSubclassOf<UCommonUserWidget> TabContentType;

	/** Reference to the content widget instance (if already created) */
	UPROPERTY(Transient)
	TObjectPtr<UWidget> CreatedTabContentWidget;
};

/** @brief Internal structure mapping a local player to their primary layout and viewport state */
USTRUCT()
struct FGameplayPrimaryLayoutViewport
{
	GENERATED_BODY()

	FGameplayPrimaryLayoutViewport() {}
	FGameplayPrimaryLayoutViewport(ULocalPlayer* InLocalPlayer, UGameplayPrimaryLayout* InPrimaryLayout, bool bIsInViewport)
		: LocalPlayer(InLocalPlayer)
		, PrimaryLayout(InPrimaryLayout)
		, bAddedToViewport(bIsInViewport)
	{
	}

public:
	/** Owner of the layout */
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> LocalPlayer = nullptr;

	/** The root layout widget */
	UPROPERTY(Transient)
	TObjectPtr<UGameplayPrimaryLayout> PrimaryLayout = nullptr;

	/** Tracking whether this layout has been added to the game viewport */
	UPROPERTY(Transient)
	bool bAddedToViewport = false;

	bool operator==(const ULocalPlayer* OtherLocalPlayer) const
	{
		return LocalPlayer == OtherLocalPlayer;
	}
};
