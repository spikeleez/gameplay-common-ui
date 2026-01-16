// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplaySetting.h"
#include "UObject/Object.h"
#include "GameplayTagContainer.h"
#include "Misc/Paths.h"
#include "Engine/LocalPlayer.h"
#include "GameplaySettingRegistry.generated.h"

enum class EGameplaySettingChangeReason : uint8;
class ULocalPlayer;
struct FGameplaySettingFilterState;

/** @brief Macro to create a dynamic data source path from a getter function and a property/function on the returned object */
#define GET_SETTINGS_FUNCTION_PATH(AccessClass, AccessFunction, SettingsClass, PropertyName)	\
		MakeShared<FGameplaySettingDataSourceDynamic>(TArray<FString>(							\
		{																						\
			GET_FUNCTION_NAME_STRING_CHECKED(AccessClass, AccessFunction),						\
			GET_FUNCTION_NAME_STRING_CHECKED(SettingsClass, PropertyName)						\
		}))

/*
#define GET_SHARED_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName) \
		GET_SETTINGS_FUNCTION_PATH(UGameplayLocalPlayer, GetSharedSettings, UGameplaySettingsShared, FunctionOrPropertyName)

#define GET_LOCAL_SETTINGS_FUNCTION_PATH(FunctionOrPropertyName) \
		GET_SETTINGS_FUNCTION_PATH(UGameplayLocalPlayer, GetLocalSettings, UGameplaySettingsLocal, FunctionOrPropertyName)
*/

/**
 * @brief Manages the registry of gameplay settings for a local player
 * 
 * This abstract class serves as the foundation for gameplay settings registries.
 * It maintains a collection of settings, provides functionality for initialization,
 * querying, and filtering settings, and handles setting-related events.
 * 
 * The registry keeps track of both top-level settings and all registered settings,
 * allowing for hierarchical organization of settings. It also manages the relationship
 * between settings and the owning local player.
 * 
 * @note This is an abstract class that must be inherited by specific implementations.
 * @see UGameplaySetting
 */
UCLASS(Abstract, BlueprintType)
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingRegistry : public UObject
{
	GENERATED_BODY()

public:
	/** Event broadcasted when a setting value changes */
	DECLARE_EVENT_TwoParams(UGameplaySettingRegistry, FGameplaySettingChanged, UGameplaySetting*, EGameplaySettingChangeReason);
	/** @brief Native event broadcast when any registered setting's value is modified */
	FGameplaySettingChanged OnSettingChangedEvent;

	/** Event broadcasted when a setting's edit condition changes */
	DECLARE_EVENT_OneParam(UGameplaySettingRegistry, FGameplaySettingEditConditionChanged, UGameplaySetting*);
	/** @brief Native event broadcast when any registered setting's visibility/editability state changes */
	FGameplaySettingEditConditionChanged OnSettingEditConditionChangedEvent;

	/** Event broadcasted when a named action is executed on a setting */
	DECLARE_EVENT_TwoParams(UGameplaySettingRegistry, FGameplaySettingNamedActionEvent, UGameplaySetting* /** Setting */, FGameplayTag /** GameActionTag */);
	/** @brief Native event broadcast when a setting action (e.g., "Reset") is triggered */
	FGameplaySettingNamedActionEvent OnSettingNamedActionEvent;

	/** Event broadcasted when navigation is requested from a setting */
	DECLARE_EVENT_OneParam(UGameplaySettingRegistry, FGameplaySettingExecuteNavigation, UGameplaySetting* /** Setting */);
	/** @brief Native event broadcast when a navigation setting (sub-page) is triggered */
	FGameplaySettingExecuteNavigation OnExecuteNavigationEvent;

public:
	UGameplaySettingRegistry(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	/**
	 * @brief Gets or creates a settings registry instance for a local player
	 * 
	 * This template function retrieves an existing registry instance or creates a new one
	 * if it doesn't exist. The registry is stored as a subobject of the local player.
	 * 
	 * @tparam RegistryClass The type of registry to retrieve (must derive from UGameplaySettingRegistry)
	 * @param InLocalPlayer The local player that owns or will own the registry
	 * @return Pointer to the registry instance, or nullptr if InLocalPlayer is invalid
	 * 
	 * @note The registry uses the class name as its unique identifier
	 * @note If a new registry is created, Initialize() is called automatically
	 * 
	 * Example usage:
	 * @code
	 * UCustomSettingRegistry* Registry = UGameplaySettingRegistry::GetRegistry<UCustomSettingRegistry>(LocalPlayer);
	 * @endcode
	 */
	template <typename RegistryClass>
	static RegistryClass* GetRegistry(const ULocalPlayer* InLocalPlayer)
	{
		if (!IsValid(InLocalPlayer))
		{
			return nullptr;
		}
		
		// We use the Class Name as the unique name for this object instance.
		const FName RegistryName = RegistryClass::StaticClass()->GetFName();
		
		RegistryClass* RegistryInstance = FindObject<RegistryClass>(InLocalPlayer, *RegistryName.ToString(), true);
		if (!RegistryInstance)
		{
			RegistryInstance = NewObject<RegistryClass>(const_cast<ULocalPlayer*>(InLocalPlayer), RegistryClass::StaticClass());
			if (RegistryInstance)
			{
				RegistryInstance->Initialize(const_cast<ULocalPlayer*>(InLocalPlayer));
			}
		}
		
		return RegistryInstance;
	}
	
	/**
	 * @brief Initializes the registry with a local player
	 * @param InLocalPlayer The local player that owns this registry
	 */
	void Initialize(ULocalPlayer* InLocalPlayer);

	/** @brief Regenerates all settings in the registry, discarding existing objects */
	virtual void Regenerate();
	
	/** @brief Checks if the registry has finished initializing all its settings
	 * @return True if initialization is complete and all settings are ready
	 */
	virtual bool IsFinishedInitializing() const;
	
	/** @brief Saves all pending changes for all registered settings to disk/persistent storage */
	virtual void SaveChanges();

	/**
	 * @brief Gets all settings that match the specified filter state
	 * @param InFilterState The filter criteria to apply
	 * @param InOutSettings Array to populate with matching settings
	 */
	void GetSettingsForFilter(const FGameplaySettingFilterState& InFilterState, TArray<UGameplaySetting*>& InOutSettings);

	/**
	 * @brief Finds a setting by its internal developer name
	 * @param InSettingDevName The developer name to search for
	 * @return Pointer to the setting if found, nullptr otherwise
	 */
	UGameplaySetting* FindSettingByDevName(const FName& InSettingDevName);

	/**
	 * @brief Finds a setting by its developer name with a type check
	 * 
	 * @tparam T The expected type of the setting (must derive from UGameplaySetting)
	 * @param InSettingDevName The developer name to search for
	 * @return Pointer to the setting cast to type T
	 * 
	 * @note This function uses check() and will crash if the setting is not found or cannot be cast to T
	 * @warning Only use this when you are certain the setting exists
	 */
	template<typename T = UGameplaySetting>
	T* FindSettingByDevNameChecked(const FName& InSettingDevName)
	{
		T* Setting = Cast<T>(FindSettingByDevName(InSettingDevName));
		check(Setting);
		return Setting;
	}

protected:
	/** Top-level settings in the registry hierarchy (usually collections/pages) */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameplaySetting>> TopLevelSettings;

	/** All registered settings (flattened list of everything in the registry) */
	UPROPERTY(Transient)
	TArray<TObjectPtr<UGameplaySetting>> RegisteredSettings;

	/** The local player that owns this registry */
	UPROPERTY(Transient)
	TObjectPtr<ULocalPlayer> OwningLocalPlayer;
	
protected:
	/**
	 * @brief Called when the registry is initialized - override this to add your settings
	 * @param InLocalPlayer The local player initializing this registry
	 * @note Must be implemented by derived classes
	 */
	virtual void OnInitialize(ULocalPlayer* InLocalPlayer) PURE_VIRTUAL(, );
	
	/**
	 * @brief Called when a setting is applied
	 * @param InSetting The setting that was applied
	 */
	virtual void OnSettingApplied(UGameplaySetting* InSetting) {  }

	/**
	 * @brief Registers a top-level setting with the registry
	 * @param InSetting The setting to register
	 */
	void RegisterSetting(UGameplaySetting* InSetting);
	
	/**
	 * @brief Registers a nested setting with the registry
	 * @param InSetting The setting to register
	 */
	void RegisterInnerSetting(UGameplaySetting* InSetting);

	// Internal event handlers
	
	/** @brief Handles setting value change events and broadcasts them via the registry */
	void HandleSettingChanged(UGameplaySetting* Setting, EGameplaySettingChangeReason Reason);
	
	/** @brief Handles setting application events */
	void HandleSettingApplied(UGameplaySetting* Setting);
	
	/** @brief Handles setting edit condition change events */
	void HandleSettingEditConditionsChanged(UGameplaySetting* Setting);
	
	/** @brief Handles named action execution events */
	void HandleSettingNamedAction(UGameplaySetting* Setting, FGameplayTag ActionTag);
	
	/** @brief Handles navigation request events */
	void HandleSettingNavigation(UGameplaySetting* Setting);
};

