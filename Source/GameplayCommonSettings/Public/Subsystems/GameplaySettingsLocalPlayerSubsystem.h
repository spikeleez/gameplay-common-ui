// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Subsystems/LocalPlayerSubsystem.h"
#include "GameplaySettingsLocalPlayerSubsystem.generated.h"

UCLASS()
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingsLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
};
