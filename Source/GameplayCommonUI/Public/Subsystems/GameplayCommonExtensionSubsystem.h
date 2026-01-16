// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayCommonExtensionSubsystem.generated.h"

class UGameplayCommonExtensionSubsystem;
struct FGameplayUIExtensionRequest;

/** @brief Match rule for extension points */
UENUM(BlueprintType)
enum class EGameplayUIExtensionAction : uint8
{
	/** An extension has been added */
	Added,
	
	/** An extension has been removed */
	Removed
};

/** @brief Native delegate for extension point updates */
DECLARE_DELEGATE_TwoParams(FGameplayExtendUIExtensionPointSignature, EGameplayUIExtensionAction ExtensionAction, const FGameplayUIExtensionRequest& ExtensionRequest);

/** 
 * @brief Internal data structure representing a UI extension 
 * 
 * An extension consists of a target point (tag), a priority, optional context, 
 * and the actual data (like a widget class).
 */
struct FGameplayUIExtension : TSharedFromThis<FGameplayUIExtension>
{
public:
	/** The extension point this extension is intended for. */
	FGameplayTag ExtensionPointTag;
	
	/** Priority that determines ordering if multiple extensions target the same point */
	int32 Priority = INDEX_NONE;
	
	/** Optional object used to filter or provide context for the extension */
	TWeakObjectPtr<UObject> ContextObject;
	
	/** The data payload for this extension (e.g., UWidget subclass or data asset) */
	TObjectPtr<UObject> Data = nullptr;
};

/** @brief A handle identifying a registered UI extension, used for unregistration */
USTRUCT(BlueprintType)
struct FGameplayUIExtensionHandle
{
	GENERATED_BODY()

	FGameplayUIExtensionHandle() 
	{}
	FGameplayUIExtensionHandle(UGameplayCommonExtensionSubsystem* InExtensionSource, const TSharedPtr<FGameplayUIExtension>& InDataPtr) 
		: ExtensionSource(InExtensionSource)
		, DataPtr(InDataPtr) 
	{}

	/** Unregisters the associated extension from the subsystem */
	void Unregister();

	/** Checks if the handle points to a valid extension */
	FORCEINLINE bool IsValid() const { return DataPtr.IsValid(); }
	
	bool operator==(const FGameplayUIExtensionHandle& Other) const { return DataPtr == Other.DataPtr; }
	bool operator!=(const FGameplayUIExtensionHandle& Other) const { return !operator==(Other); }
	friend FORCEINLINE uint32 GetTypeHash(FGameplayUIExtensionHandle Handle) { return PointerHash(Handle.DataPtr.Get()); }

private:
	/** The subsystem that created this extension */
	TWeakObjectPtr<UGameplayCommonExtensionSubsystem> ExtensionSource;
	
	/** Shared pointer to the internal extension data */
	TSharedPtr<FGameplayUIExtension> DataPtr;
	
	friend UGameplayCommonExtensionSubsystem;
};

template<>
struct TStructOpsTypeTraits<FGameplayUIExtensionHandle> : TStructOpsTypeTraitsBase2<FGameplayUIExtensionHandle>
{
	enum
	{
		WithCopy = true,  // This ensures the opaque type is copied correctly in BPs
		WithIdenticalViaEquality = true,
	};
};

/** @brief Data structure passed to extension point callbacks when an extension is added or removed */
USTRUCT(BlueprintType)
struct FGameplayUIExtensionRequest
{
	GENERATED_BODY()

public:
	/** Handle identifying the specific extension instance */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Extension")
	FGameplayUIExtensionHandle ExtensionHandle;

	/** The tag of the extension point this request is for */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Extension")
	FGameplayTag ExtensionPointTag;

	/** The priority of the extension */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Extension")
	int32 Priority = INDEX_NONE;

	/** The data associated with the extension */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Extension")
	TObjectPtr<UObject> Data = nullptr;

	/** The context object associated with the extension */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Extension")
	TObjectPtr<UObject> ContextObject = nullptr;
};

/** @brief Blueprint-friendly delegate for extension point updates */
DECLARE_DYNAMIC_DELEGATE_TwoParams(FGameplayExtendUIExtensionPointDynamicSignature, EGameplayUIExtensionAction, ExtensionAction, const FGameplayUIExtensionRequest&, ExtensionRequest);

/** @brief Rules for matching tags to extension points */
UENUM(BlueprintType)
enum class EGameplayUIExtensionPointMatch : uint8
{
	/** An exact match will only receive extensions with exactly the same point (e.g., "A.B" matches "A.B" only) */
	ExactMatch,

	/** A partial match will receive any extensions rooted in the same point (e.g., "A.B" matches "A.B.C") */
	PartialMatch
};

/** @brief Internal representation of a registered extension point (a place where UI can be extended) */
struct FGameplayUIExtensionPoint : TSharedFromThis<FGameplayUIExtensionPoint>
{
public:
	/** The tag uniquely identifying this extension point */
	FGameplayTag ExtensionPointTag;
	
	/** Optional context that must match the extension's context */
	TWeakObjectPtr<UObject> ContextObject;
	
	/** How to match tags between points and extensions */
	EGameplayUIExtensionPointMatch ExtensionPointTagMatchType = EGameplayUIExtensionPointMatch::ExactMatch;
	
	/** If non-empty, only extensions with data of these types will be accepted */
	TArray<TObjectPtr<UClass>> AllowedDataClasses;
	
	/** Native callback to invoke when an extension is added or removed */
	FGameplayExtendUIExtensionPointSignature Callback;

	/** Checks if a specific extension meets the requirements of this point */
	bool DoesExtensionPassContract(const FGameplayUIExtension* Extension) const;
};

/** @brief A handle identifying a registered extension point */
USTRUCT(BlueprintType)
struct FGameplayUIExtensionPointHandle
{
	GENERATED_BODY()

public:
	FGameplayUIExtensionPointHandle() 
	{}
	FGameplayUIExtensionPointHandle(UGameplayCommonExtensionSubsystem* InExtensionSource, const TSharedPtr<FGameplayUIExtensionPoint>& InDataPtr) 
		: ExtensionSource(InExtensionSource)
		, DataPtr(InDataPtr) 
	{}

	/** Unregisters the associated extension point from the subsystem */
	void Unregister();

	/** Checks if the handle points to a valid extension point */
	FORCEINLINE bool IsValid() const { return DataPtr.IsValid(); }
	
	bool operator==(const FGameplayUIExtensionPointHandle& Other) const { return DataPtr == Other.DataPtr; }
	bool operator!=(const FGameplayUIExtensionPointHandle& Other) const { return !operator==(Other); }
	friend uint32 GetTypeHash(const FGameplayUIExtensionPointHandle& Handle) { return PointerHash(Handle.DataPtr.Get()); }

private:
	/** The subsystem that created this extension point */
	TWeakObjectPtr<UGameplayCommonExtensionSubsystem> ExtensionSource;
	
	/** Shared pointer to the internal extension point data */
	TSharedPtr<FGameplayUIExtensionPoint> DataPtr;
	
	friend UGameplayCommonExtensionSubsystem;
};

template<>
struct TStructOpsTypeTraits<FGameplayUIExtensionPointHandle> : TStructOpsTypeTraitsBase2<FGameplayUIExtensionPointHandle>
{
	enum
	{
		WithCopy = true,  // This ensures the opaque type is copied correctly in BPs
		WithIdenticalViaEquality = true,
	};
};

/**
 * @brief Subsystem for managing dynamic UI extensions
 * 
 * This subsystem allows widgets or other systems to register "Extension Points" 
 * identified by Gameplay Tags. Other parts of the game can then "extend" these 
 * points by providing widgets or data. 
 * 
 * For example: A HUD might register an extension point "HUD.BottomLeft". 
 * A gameplay ability system might then register a "CooldownWidget" to that point 
 * without the HUD having to know anything about the ability system.
 */
UCLASS(meta=(DisplayName="Gameplay Common Extension Subsystem"))
class GAMEPLAYCOMMONUI_API UGameplayCommonExtensionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()
	
public:
	UGameplayCommonExtensionSubsystem();
	
	//~Begin USubsystem
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	//~End of USubsystem
	
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	
	/** Native: Registers a new extension point */
	FGameplayUIExtensionPointHandle RegisterExtensionPointInternal(const FGameplayTag& ExtensionPointTag, EGameplayUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses, const FGameplayExtendUIExtensionPointSignature& ExtensionCallback);
	
	/** Native: Registers a new extension point for a specific context object */
	FGameplayUIExtensionPointHandle RegisterExtensionPointForContextInternal(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, EGameplayUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses, FGameplayExtendUIExtensionPointSignature ExtensionCallback);

	/** Native: Registers a widget class as an extension to a point */
	FGameplayUIExtensionHandle RegisterExtensionAsWidgetInternal(const FGameplayTag& ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);
	
	/** Native: Registers a widget class for a specific context object */
	FGameplayUIExtensionHandle RegisterExtensionAsWidgetForContextInternal(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, TSubclassOf<UUserWidget> WidgetClass, int32 Priority);
	
	/** Native: Registers arbitrary data as an extension */
	FGameplayUIExtensionHandle RegisterExtensionAsDataInternal(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority);

	/** Unregisters a previously registered extension */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Extension")
	void UnregisterExtension(const FGameplayUIExtensionHandle& ExtensionHandle);

	/** Unregisters a previously registered extension point */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Extension")
	void UnregisterExtensionPoint(const FGameplayUIExtensionPointHandle& ExtensionPointHandle);
	
protected:
	/** Helper to convert internal extension data into a request structure for callbacks */
	FGameplayUIExtensionRequest CreateExtensionRequest(const TSharedPtr<FGameplayUIExtension>& Extension);
	
	/** Informs an extension point about all currently registered extensions that match its criteria */
	void NotifyExtensionPointOfExtensions(const TSharedPtr<FGameplayUIExtensionPoint>& ExtensionPoint);
	
	/** Informs all registered extension points about a new or removed extension */
	void NotifyExtensionPointsOfExtension(EGameplayUIExtensionAction ExtensionAction, const TSharedPtr<FGameplayUIExtension>& Extension);
	
	/** Blueprint: Registers a new extension point */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Extension", meta = (DisplayName = "Register Extension Point"))
	FGameplayUIExtensionPointHandle RegisterExtensionPoint(FGameplayTag ExtensionPointTag, EGameplayUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses, FGameplayExtendUIExtensionPointDynamicSignature ExtensionCallback);
	
	/** Blueprint: Registers a widget class as an extension to a point */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Extension", meta = (DisplayName = "Register Extension (Widget)"))
	FGameplayUIExtensionHandle RegisterExtensionAsWidget(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority = -1);
	
	/**
	 * Registers the widget (as data) for a specific player context. This means the extension points will receive a UIExtensionForPlayer data object
	 * that they can look at to determine if it's for whatever they consider their player.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Extension", meta = (DisplayName = "Register Extension (Widget For Context)"))
	FGameplayUIExtensionHandle RegisterExtensionAsWidgetForContext(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, UObject* ContextObject, int32 Priority = -1);

	/**
	 * Registers the extension as data for any extension point that can make use of it.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Extension", meta = (DisplayName = "Register Extension (Data)"))
	FGameplayUIExtensionHandle RegisterExtensionAsData(FGameplayTag ExtensionPointTag, UObject* Data, int32 Priority = -1);

	/**
	 * Registers the extension as data for a specific context object.
	 */
	UFUNCTION(BlueprintCallable, BlueprintCosmetic, Category = "Gameplay UI Extension", meta = (DisplayName = "Register Extension (Data For Context)"))
	FGameplayUIExtensionHandle RegisterExtensionAsDataForContext(FGameplayTag ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority = -1);
	
private:
	typedef TArray<TSharedPtr<FGameplayUIExtensionPoint>> FExtensionPointList;
	/** Map of tags to a list of registered points at that tag */
	TMap<FGameplayTag, FExtensionPointList> ExtensionPointMap;

	typedef TArray<TSharedPtr<FGameplayUIExtension>> FExtensionList;
	/** Map of tags to a list of registered extensions at that tag */
	TMap<FGameplayTag, FExtensionList> ExtensionMap;
};

