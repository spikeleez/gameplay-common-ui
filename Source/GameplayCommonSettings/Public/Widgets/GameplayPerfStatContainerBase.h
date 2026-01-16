// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "CommonUserWidget.h"
#include "Misc/GameplayCommonPerformanceSettings.h"
#include "GameplayPerfStatContainerBase.generated.h"

class UObject;
struct FFrame;

/**
 * @brief Widget container that manages multiple performance statistic widgets
 *
 * This panel is responsible for aggregating various performance stat widgets (like FPS, Ping, etc.)
 * and controlling their visibility globally based on the user's performance settings.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplayPerfStatContainerBase : public UCommonUserWidget
{
	GENERATED_BODY()
	
public:
	UGameplayPerfStatContainerBase(const FObjectInitializer& ObjectInitializer);

	// ~Begin UUserWidget interface
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// ~End of UUserWidget interface

	/**
	 * @brief Refreshes the visibility of all child stat widgets
	 * 
	 * Iterates through children and hides/shows them based on the current 
	 * user preferences for performance stat display.
	 */
	UFUNCTION(BlueprintCallable, Category = "Performance Stats")
	void UpdateVisibilityOfChildren();

protected:
	/**
	 * @brief Filter for which display modes this container handles
	 * 
	 * For example, one container might only display Text stats, while another displays Graph stats.
	 */
	UPROPERTY(EditAnywhere, Category = "Display")
	EGameplayPerformanceStatDisplayMode StatDisplayModeFilter = EGameplayPerformanceStatDisplayMode::TextAndGraph;
};