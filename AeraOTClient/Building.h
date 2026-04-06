#pragma once

#include "SDK/FortniteGame_classes.hpp"
#include "SDK/FortniteGame_parameters.hpp"

class BuildingAera
{
public:
	static SDK::TArray<SDK::AActor*> GetAllBuildingActors();
	static void OnDamageServer(SDK::Params::BuildingActor_OnDamageServer* NewParams);
	static void OnServerCreateBuildingActor(SDK::Params::FortPlayerController_ServerCreateBuildingActor* NewParams);
};