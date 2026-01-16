// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingFilterState.h"
#include "EditCondition/GameplaySettingEditableState.h"
#include "Framework/GameplaySetting.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingFilterState)

class FGameplayGameSettingFilterExpressionContext : public ITextFilterExpressionContext
{
public:
	explicit FGameplayGameSettingFilterExpressionContext(const UGameplaySetting& InSetting)
		: Setting(InSetting)
	{
	}

	virtual bool TestBasicStringExpression(const FTextFilterString& InValue, const ETextFilterTextComparisonMode InTextComparisonMode) const override
	{
		return TextFilterUtils::TestBasicStringExpression(Setting.GetDescriptionPlainText(), InValue, InTextComparisonMode);
	}

	virtual bool TestComplexExpression(const FName& InKey, const FTextFilterString& InValue, const ETextFilterComparisonOperation InComparisonOperation, const ETextFilterTextComparisonMode InTextComparisonMode) const override
	{
		return false;
	}

private:
	/** Setting being filtered. */
	const UGameplaySetting& Setting;
};

FGameplaySettingFilterState::FGameplaySettingFilterState()
	: bIncludeDisabled(true)
	, bIncludeHidden(false)
	, bIncludeResetable(true)
	, bIncludeNestedPages(false)
	, SearchTextEvaluator(ETextFilterExpressionEvaluatorMode::BasicString)
{
	
}

void FGameplaySettingFilterState::SetSearchText(const FString& InSearchText)
{
	SearchTextEvaluator.SetFilterText(FText::FromString(InSearchText));
}

bool FGameplaySettingFilterState::DoesSettingPassFilter(const UGameplaySetting& InSetting) const
{
	const FGameplaySettingEditableState& EditableState = InSetting.GetEditState();

	if (!bIncludeHidden && !EditableState.IsVisible())
	{
		return false;
	}
	
	if (!bIncludeDisabled && !EditableState.IsEnabled())
	{
		return false;
	}
	
	if (!bIncludeResetable && !EditableState.IsResetable())
	{
		return false;
	}

	if (SettingAllowList.Num() > 0)
	{
		if (!SettingAllowList.Contains(&InSetting))
		{
			bool bAllowed = false;
			const UGameplaySetting* NextSetting = &InSetting;
			while (const UGameplaySetting* Parent = NextSetting->GetSettingParent())
			{
				if (SettingAllowList.Contains(Parent))
				{
					bAllowed = true;
					break;
				}
				NextSetting = Parent;
			}
			
			if (!bAllowed)
			{
				return false;
			}
		}
	}

	// Always search text last, it's generally the most expensive filter.
	if (!SearchTextEvaluator.TestTextFilter(FGameplayGameSettingFilterExpressionContext(InSetting)))
	{
		return false;
	}

	return true;
}

void FGameplaySettingFilterState::AddSettingToRootList(UGameplaySetting* InSetting)
{
	SettingAllowList.Add(InSetting);
	SettingRootList.Add(InSetting);
}

void FGameplaySettingFilterState::AddSettingToAllowList(UGameplaySetting* InSetting)
{
	SettingAllowList.Add(InSetting);
}
