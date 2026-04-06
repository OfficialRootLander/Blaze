#pragma once
#ifndef PLAYERCONTROLLER_H
#define PLAYERCONTROLLER_H

#include "SDK/FortniteGame_classes.hpp"

class PlayerControllerAera
{
public:
	static void TeleportPlayer(SDK::AFortPlayerController* CurrentController, SDK::FVector Location, SDK::FRotator Rotation);
	static void PlacePlayerAtStartSpot(SDK::AFortPlayerController* CurrentController);
	static SDK::AFortPlayerController* GetLocalPlayerController();
	static void CleanPlayer(SDK::AFortPlayerController* CurrentController);
	static void StartClientController();
};

extern SDK::AFortPlayerController* SavedController;

void DoSomthing();
#endif