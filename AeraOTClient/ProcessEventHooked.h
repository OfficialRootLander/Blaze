#pragma once
#ifndef PROCESSEVENTBASIC_H
#define PROCESSEVENTBASIC_H

#include "SDK/FortniteGame_classes.hpp"
#include "SDK/FortniteGame_parameters.hpp"
#include "SDK/GameplayAbilities_classes.hpp"
#include "SDK/Engine_classes.hpp"
#include "SDK/Engine_parameters.hpp"

// Flag to indicate when the world is ready
extern bool PIsWorldReady;

inline void* (__fastcall* ProcessEvent)(void* _Object, void* Object, void* Function, void* Params);

// Original ProcessEvent pointer
extern void* (__fastcall* ProcessEventOriginal)(SDK::UObject* _Object, SDK::UObject* Object, SDK::UFunction* Function, void* Params);

// Hook typedef
using ProcessEvent_t = void* (__fastcall*)(SDK::UObject* _Object, SDK::UObject* Object, SDK::UFunction* Function, void* Params);

class ProcessEventBasic
{
public:
    // Our hook
    static void* __fastcall ProcessEventHook(SDK::UObject* _Object, SDK::UObject* Object, SDK::UFunction* Function, void* Params);
};

extern bool bReadyToSpawnHorde;

#endif
