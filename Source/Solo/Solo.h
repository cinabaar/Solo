// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(LogSolo, Log, All)

#ifndef CONTROLLER_SWAPPING
#define CONTROLLER_SWAPPING 1
#endif

#ifndef SOLO_CONSOLE_UI
/** Set to 1 to pretend we're building for console even on a PC, for testing purposes */
#define SOLO_SIMULATE_CONSOLE_UI	1

#if PLATFORM_PS4 || PLATFORM_SWITCH || SOLO_SIMULATE_CONSOLE_UI
#define SOLO_CONSOLE_UI 1
#else
#define SOLO_CONSOLE_UI 0
#endif
#endif


template<typename T>
static FString EnumToString(const FString& enumName, const T value)
{
	UEnum* pEnum = FindObject<UEnum>(ANY_PACKAGE, *enumName);
	return *(pEnum ? pEnum->GetNameStringByIndex(static_cast<uint8>(value)) : "null");
}
#ifndef ENUM_TO_STRING
#define ENUM_TO_STRING(_enumType, _value) EnumToString<_enumType>(TEXT(#_enumType), _value)
#endif