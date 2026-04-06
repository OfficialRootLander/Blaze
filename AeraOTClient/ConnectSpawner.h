#pragma once
#include "Utils.h"

class ConnectSpawner
{
public:
	static void SpawnClientPlayerController(SDK::FVector SpawnLocation = SDK::FVector(0, 0, 0), SDK::FRotator Rotation = SDK::FRotator(0, 0, 0));
};