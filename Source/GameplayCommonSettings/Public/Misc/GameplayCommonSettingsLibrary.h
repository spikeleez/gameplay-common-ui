#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/LocalPlayer.h"
#include "Engine/World.h"
#include "GameplayCommonSettingsLibrary.generated.h"

class UGameplaySettingsLocal;
class UGameplaySettingsShared;

UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplayCommonSettingsLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintPure, Category = "Gameplay Settings Library")
	static UGameplaySettingsShared* GetSettingsShared(const ULocalPlayer* LocalPlayer);
	
	UFUNCTION(BlueprintPure, Category = "Gameplay Settings Library")
	static UGameplaySettingsLocal* GetLocalSettings(const ULocalPlayer* LocalPlayer);
};
