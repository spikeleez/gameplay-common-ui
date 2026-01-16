// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Widgets/GameplaySettingPressAnyKey.h"
#include "GameplaySettingKeyAlreadyBound.generated.h"

class UCommonTextBlock;

/**
 * @brief Feedback widget shown when a user tries to bind a key that's already in use
 *
 * This widget extends the "Press Any Key" flow to include specific feedback about 
 * binding conflicts, allowing the user to see what they are about to overwrite.
 */
UCLASS(Abstract, BlueprintType, Blueprintable, meta=(DisableNativeTick))
class GAMEPLAYCOMMONSETTINGS_API UGameplaySettingKeyAlreadyBound : public UGameplaySettingPressAnyKey
{
	GENERATED_BODY()

public:
	/** @brief Sets the warning message explaining the key conflict */
	void SetWarningText(const FText& InText);
	
	/** @brief Sets the text for the cancellation instruction */
	void SetCancelText(const FText& InText);

protected:
	/** Text block for the conflict warning */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonTextBlock> Text_Warning;

	/** Text block for the cancel instructions */
	UPROPERTY(BlueprintReadOnly, Category="Designer", meta=(BindWidget, BlueprintProtected=true, AllowPrivateAccess=true))
	TObjectPtr<UCommonTextBlock> Text_Cancel;
};