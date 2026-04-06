#pragma once
#include "Utils.h"
#include "SDK/HuskPawn_classes.hpp"
#include "SDK/HuskPawn_parameters.hpp"
#include "SDK/HuskPawn_Bombshell_classes.hpp"
#include "SDK/HuskPawn_Bombshell_Poison_classes.hpp"
#include "SDK/FlingerPawn_classes.hpp"
#include "SDK/FlingerVimBlueprint_New_classes.hpp"
//Husky removed for now and beehive death effects can crash so be careful
enum HUSK_AERA_TYPE : uint8_t
{
    NORMAL,
    HUSK_BOMBSHELL,
    HUSK_BOMBSHELL_POISON,
    HUSK_BEEHIVE,
    HUSK_DWARF,
    HUSK_DWARF_FIRE,
    HUSK_DWARF_ICE,
    HUSK_DWARF_LIGHTNING,
    HUSK_FIRE,
    HUSK_ICE,
    HUSK_LIGHTNING,
    HUSK_PITCHER,
    FLINGER,
    HALLOWEEN_WEREWOLF,
    BLASTER
};

class HuskAera
{
public:
    static SDK::UClass* GetHuskBluePrintFast(std::string FullBluePrintClassName);
	static SDK::UClass* GetClassOfHuskType(HUSK_AERA_TYPE TYPE);
	static void SpawnHusk(HUSK_AERA_TYPE HuskType = HUSK_AERA_TYPE::NORMAL,SDK::FVector Location = SDK::FVector(0.f, 0.f, 0.f), SDK::FRotator Rotation = SDK::FRotator(0.f, 0.f, 0.f));
	static void HuskTick(void* _Object, float DeltaTime);
	static void SpawnTestHorde(SDK::FVector HordeLocation, int HusksToSpawn);
    static void FlingerTick(void* Object);
    static void InitHusks();
    static void SpawnHuskDirect(std::string ClassPath, SDK::FVector Location, SDK::FRotator Rotation);
};