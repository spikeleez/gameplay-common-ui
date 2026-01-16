// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "GameplaySettingEditableState.h"
#include "GameplaySettingEditCondition.h"

class ULocalPlayer;

/**
 * @brief An inline edit condition that uses a lambda function for evaluation
 * 
 * This class allows for quick creation of edit conditions without defining a new class.
 * It stores a TFunction that is executed during the GatherEditState phase.
 */
class FGameplaySettingWhenCondition : public FGameplaySettingEditCondition
{
public:
	/**
	 * @brief Constructs an inline edit condition
	 * @param InInlineEditCondition The lambda function to execute for evaluation
	 */
	FGameplaySettingWhenCondition(TFunction<void(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState&)>&& InInlineEditCondition)
		: InlineEditCondition(InInlineEditCondition)
	{}

	// ~Begin FGameplaySettingEditCondition interface
	virtual void GatherEditState(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState) const override
	{
		InlineEditCondition(InLocalPlayer, InOutEditState);
	}
	
	virtual FString ToString() const override
	{
		return TEXT("Inline Edit Condition");
	}
	// ~End of FGameplaySettingEditCondition interface

private:
	/** The stored lambda function for evaluation */
	TFunction<void(const ULocalPlayer* InLocalPlayer, FGameplaySettingEditableState& InOutEditState)> InlineEditCondition;
};