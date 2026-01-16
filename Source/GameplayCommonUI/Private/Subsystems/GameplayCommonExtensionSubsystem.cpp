// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Subsystems/GameplayCommonExtensionSubsystem.h"
#include "Blueprint/UserWidget.h"

DEFINE_LOG_CATEGORY_STATIC(LogGameplayUIExtensionSubsystem, Log, All);

//=========================================================
// FGameplayUIExtensionHandle
//=========================================================

void FGameplayUIExtensionHandle::Unregister()
{
	if (UGameplayCommonExtensionSubsystem* ExtensionSourcePtr = ExtensionSource.Get())
	{
		ExtensionSourcePtr->UnregisterExtension(*this);
	}
}

//=========================================================
// FGameplayUIExtensionPoint
//=========================================================

bool FGameplayUIExtensionPoint::DoesExtensionPassContract(const FGameplayUIExtension* Extension) const
{
	if (UObject* DataPtr = Extension->Data)
	{
		const bool bMatchesContext = 
			(ContextObject.IsExplicitlyNull() && Extension->ContextObject.IsExplicitlyNull()) 
			|| ContextObject == Extension->ContextObject;

		// Make sure the contexts match.
		if (bMatchesContext)
		{
			// The data can either be the literal class of the data type, or an instance of the class type.
			const UClass* DataClass = DataPtr->IsA(UClass::StaticClass()) ? Cast<UClass>(DataPtr) : DataPtr->GetClass();
			for (const UClass* AllowedDataClass : AllowedDataClasses)
			{
				if (DataClass->IsChildOf(AllowedDataClass) || DataClass->ImplementsInterface(AllowedDataClass))
				{
					return true;
				}
			}
		}
	}

	return false;
}

void FGameplayUIExtensionPointHandle::Unregister()
{
	if (UGameplayCommonExtensionSubsystem* ExtensionSourcePtr = ExtensionSource.Get())
	{
		ExtensionSourcePtr->UnregisterExtensionPoint(*this);
	}
}

//=========================================================
// UGameplayUIExtensionSubsystem
//=========================================================

UGameplayCommonExtensionSubsystem::UGameplayCommonExtensionSubsystem()
{
}

void UGameplayCommonExtensionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
}

void UGameplayCommonExtensionSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGameplayCommonExtensionSubsystem::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	Super::AddReferencedObjects(InThis, Collector);

	if (UGameplayCommonExtensionSubsystem* ExtensionSubsystem = Cast<UGameplayCommonExtensionSubsystem>(InThis))
	{
		for (auto MapIt = ExtensionSubsystem->ExtensionPointMap.CreateIterator(); MapIt; ++MapIt)
		{
			for (const TSharedPtr<FGameplayUIExtensionPoint>& ValueElement : MapIt.Value())
			{
				Collector.AddReferencedObjects(ValueElement->AllowedDataClasses);
			}
		}

		for (auto MapIt = ExtensionSubsystem->ExtensionMap.CreateIterator(); MapIt; ++MapIt)
		{
			for (const TSharedPtr<FGameplayUIExtension>& ValueElement : MapIt.Value())
			{
				Collector.AddReferencedObject(ValueElement->Data);
			}
		}
	}
}

FGameplayUIExtensionPointHandle UGameplayCommonExtensionSubsystem::RegisterExtensionPointInternal(const FGameplayTag& ExtensionPointTag, EGameplayUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses, const FGameplayExtendUIExtensionPointSignature& ExtensionCallback)
{
	return RegisterExtensionPointForContextInternal(ExtensionPointTag, nullptr, ExtensionPointTagMatchType, AllowedDataClasses, ExtensionCallback);
}

FGameplayUIExtensionPointHandle UGameplayCommonExtensionSubsystem::RegisterExtensionPointForContextInternal(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, EGameplayUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses, FGameplayExtendUIExtensionPointSignature ExtensionCallback)
{
	if (!ExtensionPointTag.IsValid())
	{
		UE_LOG(LogGameplayUIExtensionSubsystem, Warning, TEXT("Trying to register an invalid extension point."));
		return FGameplayUIExtensionPointHandle();
	}

	if (!ExtensionCallback.IsBound())
	{
		UE_LOG(LogGameplayUIExtensionSubsystem, Warning, TEXT("Trying to register an invalid extension point."));
		return FGameplayUIExtensionPointHandle();
	}

	if (AllowedDataClasses.Num() == 0)
	{
		UE_LOG(LogGameplayUIExtensionSubsystem, Warning, TEXT("Trying to register an invalid extension point."));
		return FGameplayUIExtensionPointHandle();
	}

	FExtensionPointList& List = ExtensionPointMap.FindOrAdd(ExtensionPointTag);

	const TSharedPtr<FGameplayUIExtensionPoint>& Entry = List.Add_GetRef(MakeShared<FGameplayUIExtensionPoint>());
	Entry->ExtensionPointTag = ExtensionPointTag;
	Entry->ContextObject = ContextObject;
	Entry->ExtensionPointTagMatchType = ExtensionPointTagMatchType;
	Entry->AllowedDataClasses = AllowedDataClasses;
	Entry->Callback = MoveTemp(ExtensionCallback);

	UE_LOG(LogGameplayUIExtensionSubsystem, Verbose, TEXT("Extension Point [%s] Registered"), *ExtensionPointTag.ToString());

	NotifyExtensionPointOfExtensions(Entry);

	return FGameplayUIExtensionPointHandle(this, Entry);
}

FGameplayUIExtensionHandle UGameplayCommonExtensionSubsystem::RegisterExtensionAsWidgetInternal(const FGameplayTag& ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority)
{
	return RegisterExtensionAsDataInternal(ExtensionPointTag, nullptr, WidgetClass, Priority);
}

FGameplayUIExtensionHandle UGameplayCommonExtensionSubsystem::RegisterExtensionAsWidgetForContextInternal(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, TSubclassOf<UUserWidget> WidgetClass, int32 Priority)
{
	return RegisterExtensionAsDataInternal(ExtensionPointTag, ContextObject, WidgetClass, Priority);
}

FGameplayUIExtensionHandle UGameplayCommonExtensionSubsystem::RegisterExtensionAsDataInternal(const FGameplayTag& ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority)
{
	if (!ExtensionPointTag.IsValid())
	{
		UE_LOG(LogGameplayUIExtensionSubsystem, Warning, TEXT("Trying to register an invalid extension."));
		return FGameplayUIExtensionHandle();
	}

	if (!Data)
	{
		UE_LOG(LogGameplayUIExtensionSubsystem, Warning, TEXT("Trying to register an invalid extension."));
		return FGameplayUIExtensionHandle();
	}

	FExtensionList& List = ExtensionMap.FindOrAdd(ExtensionPointTag);

	const TSharedPtr<FGameplayUIExtension>& Entry = List.Add_GetRef(MakeShared<FGameplayUIExtension>());
	Entry->ExtensionPointTag = ExtensionPointTag;
	Entry->ContextObject = ContextObject;
	Entry->Data = Data;
	Entry->Priority = Priority;

	if (ContextObject)
	{
		UE_LOG(LogGameplayUIExtensionSubsystem, Verbose, TEXT("Extension [%s] @ [%s] Registered"), *GetNameSafe(Data), *ExtensionPointTag.ToString());
	}
	else
	{
		UE_LOG(LogGameplayUIExtensionSubsystem, Verbose, TEXT("Extension [%s] for [%s] @ [%s] Registered"), *GetNameSafe(Data), *GetNameSafe(ContextObject), *ExtensionPointTag.ToString());
	}

	NotifyExtensionPointsOfExtension(EGameplayUIExtensionAction::Added, Entry);

	return FGameplayUIExtensionHandle(this, Entry);
}

void UGameplayCommonExtensionSubsystem::UnregisterExtension(const FGameplayUIExtensionHandle& ExtensionHandle)
{
	if (ExtensionHandle.IsValid())
	{
		checkf(ExtensionHandle.ExtensionSource == this, TEXT("Trying to unregister an extension that's not from this extension subsystem."));

		const TSharedPtr<FGameplayUIExtension> Extension = ExtensionHandle.DataPtr;
		if (FExtensionList* ListPtr = ExtensionMap.Find(Extension->ExtensionPointTag))
		{
			if (Extension->ContextObject.IsExplicitlyNull())
			{
				UE_LOG(LogGameplayUIExtensionSubsystem, Verbose, TEXT("Extension [%s] @ [%s] Unregistered"), *GetNameSafe(Extension->Data), *Extension->ExtensionPointTag.ToString());
			}
			else
			{
				UE_LOG(LogGameplayUIExtensionSubsystem, Verbose, TEXT("Extension [%s] for [%s] @ [%s] Unregistered"), *GetNameSafe(Extension->Data), *GetNameSafe(Extension->ContextObject.Get()), *Extension->ExtensionPointTag.ToString());
			}

			NotifyExtensionPointsOfExtension(EGameplayUIExtensionAction::Removed, Extension);

			ListPtr->RemoveSwap(Extension);
			
			if (ListPtr->Num() == 0)
			{
				ExtensionMap.Remove(Extension->ExtensionPointTag);
			}
		}
	}
	else
	{
		UE_LOG(LogGameplayUIExtensionSubsystem, Warning, TEXT("Trying to unregister an invalid Handle."));
	}
}

void UGameplayCommonExtensionSubsystem::UnregisterExtensionPoint(const FGameplayUIExtensionPointHandle& ExtensionPointHandle)
{
	if (ExtensionPointHandle.IsValid())
	{
		check(ExtensionPointHandle.ExtensionSource == this);

		const TSharedPtr<FGameplayUIExtensionPoint> ExtensionPoint = ExtensionPointHandle.DataPtr;
		if (FExtensionPointList* ListPtr = ExtensionPointMap.Find(ExtensionPoint->ExtensionPointTag))
		{
			UE_LOG(LogGameplayUIExtensionSubsystem, Verbose, TEXT("Extension Point [%s] Unregistered"), *ExtensionPoint->ExtensionPointTag.ToString());

			ListPtr->RemoveSwap(ExtensionPoint);
			if (ListPtr->Num() == 0)
			{
				ExtensionPointMap.Remove(ExtensionPoint->ExtensionPointTag);
			}
		}
	}
	else
	{
		UE_LOG(LogGameplayUIExtensionSubsystem, Warning, TEXT("Trying to unregister an invalid Handle."));
	}
}

FGameplayUIExtensionRequest UGameplayCommonExtensionSubsystem::CreateExtensionRequest(const TSharedPtr<FGameplayUIExtension>& Extension)
{
	FGameplayUIExtensionRequest Request;
	Request.ExtensionHandle = FGameplayUIExtensionHandle(this, Extension);
	Request.ExtensionPointTag = Extension->ExtensionPointTag;
	Request.Priority = Extension->Priority;
	Request.Data = Extension->Data;
	Request.ContextObject = Extension->ContextObject.Get();

	return Request;
}

void UGameplayCommonExtensionSubsystem::NotifyExtensionPointOfExtensions(const TSharedPtr<FGameplayUIExtensionPoint>& ExtensionPoint)
{
	for (FGameplayTag Tag = ExtensionPoint->ExtensionPointTag; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const FExtensionList* ListPtr = ExtensionMap.Find(Tag))
		{
			// Copy in case there are removals while handling callbacks
			FExtensionList ExtensionArray(*ListPtr);

			for (const TSharedPtr<FGameplayUIExtension>& Extension : ExtensionArray)
			{
				if (ExtensionPoint->DoesExtensionPassContract(Extension.Get()))
				{
					FGameplayUIExtensionRequest Request = CreateExtensionRequest(Extension);
					ExtensionPoint->Callback.ExecuteIfBound(EGameplayUIExtensionAction::Added, Request);
				}
			}
		}

		if (ExtensionPoint->ExtensionPointTagMatchType == EGameplayUIExtensionPointMatch::ExactMatch)
		{
			break;
		}
	}
}

void UGameplayCommonExtensionSubsystem::NotifyExtensionPointsOfExtension(EGameplayUIExtensionAction ExtensionAction, const TSharedPtr<FGameplayUIExtension>& Extension)
{
	bool bOnInitialTag = true;
	for (FGameplayTag Tag = Extension->ExtensionPointTag; Tag.IsValid(); Tag = Tag.RequestDirectParent())
	{
		if (const FExtensionPointList* ListPtr = ExtensionPointMap.Find(Tag))
		{
			// Copy in case there are removals while handling callbacks
			FExtensionPointList ExtensionPointArray(*ListPtr);

			for (const TSharedPtr<FGameplayUIExtensionPoint>& ExtensionPoint : ExtensionPointArray)
			{
				if (bOnInitialTag || (ExtensionPoint->ExtensionPointTagMatchType == EGameplayUIExtensionPointMatch::PartialMatch))
				{
					if (ExtensionPoint->DoesExtensionPassContract(Extension.Get()))
					{
						FGameplayUIExtensionRequest Request = CreateExtensionRequest(Extension);
						ExtensionPoint->Callback.ExecuteIfBound(ExtensionAction, Request);
					}
				}
			}
		}
		
		bOnInitialTag = false;
	}
}

FGameplayUIExtensionPointHandle UGameplayCommonExtensionSubsystem::RegisterExtensionPoint(FGameplayTag ExtensionPointTag, EGameplayUIExtensionPointMatch ExtensionPointTagMatchType, const TArray<UClass*>& AllowedDataClasses, FGameplayExtendUIExtensionPointDynamicSignature ExtensionCallback)
{
	FGameplayUIExtensionPointHandle ExtensionPointHandle = RegisterExtensionPointInternal(ExtensionPointTag, ExtensionPointTagMatchType, AllowedDataClasses, FGameplayExtendUIExtensionPointSignature::CreateWeakLambda(
	ExtensionCallback.GetUObject(), [this, ExtensionCallback](EGameplayUIExtensionAction Action, const FGameplayUIExtensionRequest& Request) 
	{ 
		ExtensionCallback.ExecuteIfBound(Action, Request);
	}));
	
	return ExtensionPointHandle;
}

FGameplayUIExtensionHandle UGameplayCommonExtensionSubsystem::RegisterExtensionAsWidget(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, int32 Priority)
{
	return RegisterExtensionAsWidgetInternal(ExtensionPointTag, WidgetClass, Priority);
}

FGameplayUIExtensionHandle UGameplayCommonExtensionSubsystem::RegisterExtensionAsWidgetForContext(FGameplayTag ExtensionPointTag, TSubclassOf<UUserWidget> WidgetClass, UObject* ContextObject, int32 Priority)
{
	if (ContextObject)
	{
		return RegisterExtensionAsWidgetForContextInternal(ExtensionPointTag, ContextObject, WidgetClass, Priority);
	}
	
	FFrame::KismetExecutionMessage(TEXT("A null ContextObject was passed to Register Extension (Widget For Context)"), ELogVerbosity::Error);
	return FGameplayUIExtensionHandle();
}

FGameplayUIExtensionHandle UGameplayCommonExtensionSubsystem::RegisterExtensionAsData(FGameplayTag ExtensionPointTag, UObject* Data, int32 Priority)
{
	return RegisterExtensionAsDataInternal(ExtensionPointTag, nullptr, Data, Priority);
}

FGameplayUIExtensionHandle UGameplayCommonExtensionSubsystem::RegisterExtensionAsDataForContext(FGameplayTag ExtensionPointTag, UObject* ContextObject, UObject* Data, int32 Priority)
{
	if (ContextObject)
	{
		return RegisterExtensionAsDataInternal(ExtensionPointTag, ContextObject, Data, Priority);
	}
	
	FFrame::KismetExecutionMessage(TEXT("A null ContextObject was passed to Register Extension (Data For Context)"), ELogVerbosity::Error);
	return FGameplayUIExtensionHandle();
}
