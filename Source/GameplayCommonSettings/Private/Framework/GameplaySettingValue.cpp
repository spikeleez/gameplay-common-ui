// Copyright Spike Plugins 2026. All Rights Reserved.

#include "Framework/GameplaySettingValue.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(GameplaySettingValue)

UGameplaySettingValue::UGameplaySettingValue(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Values will report to analytics.
	bReportAnalytics = true;
}

void UGameplaySettingValue::OnInitialized()
{
	Super::OnInitialized();

#if !UE_BUILD_SHIPPING
	ensureAlwaysMsgf(!DescriptionRichText.IsEmpty() || DynamicDetails.IsBound(),
		TEXT("You must provide a description or it must specify a dynamic details function for settings with values."));
#endif

	StoreInitial();
}
