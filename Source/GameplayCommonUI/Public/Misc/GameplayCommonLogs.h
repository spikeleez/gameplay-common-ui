// Copyright Spike Plugins 2026. All Rights Reserved.

#pragma once

#include "Logging/LogMacros.h"

/** @brief Log category for general Gameplay Common UI operations */
GAMEPLAYCOMMONUI_API DECLARE_LOG_CATEGORY_EXTERN(LogGameplayCommonUI, Log, All);

/** @brief Log category for Gameplay Common Settings operations */
GAMEPLAYCOMMONUI_API DECLARE_LOG_CATEGORY_EXTERN(LogGameplayCommonSettings, Log, All);

/** @brief Internal macro to get the current file name for logging */
#define COMMON_PRINT_FILE (FString(FPaths::GetCleanFilename(TEXT(__FILE__))))

/** @brief Internal macro to get the current function name for logging */
#define COMMON_PRINT_FUNC (FString(__FUNCTION__))

/** @brief Internal macro to get the current line number as string for logging */
#define COMMON_PRINT_LINE (FString::FromInt(__LINE__))

/** @brief Internal macro to format a source location string (Function [File:Line]) */
#define COMMON_LOGS_LINE (COMMON_PRINT_FUNC + TEXT(" [") + COMMON_PRINT_FILE + TEXT(":") + COMMON_PRINT_LINE + TEXT("]"))

/** @brief Standard UI logging macro including source location information */
#define COMMON_UI_LOG(Verbosity, Format, ...) UE_LOG(LogGameplayCommonUI, Verbosity, TEXT("%s %s"), *COMMON_LOGS_LINE, *FString::Printf(Format, ##__VA_ARGS__))

/** @brief Standard Settings logging macro including source location information */
#define COMMON_SETTINGS_LOG(Verbosity, Format, ...) UE_LOG(LogGameplayCommonSettings, Verbosity, TEXT("%s %s"), *COMMON_LOGS_LINE, *FString::Printf(Format, ##__VA_ARGS__))

/** @brief Generic logging macro that accepts a custom category and includes source location information */
#define COMMON_LOG(CategoryName, Verbosity, Format, ...) UE_LOG(CategoryName, Verbosity, TEXT("%s %s"), *COMMON_LOGS_LINE, *FString::Printf(Format, ##__VA_ARGS__))
