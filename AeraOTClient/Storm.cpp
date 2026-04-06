#include "Storm.h"
#include "PlayerController.h"

void StormAera::StartBasicStorm()
{
	SDK::UKismetSystemLibrary::ExecuteConsoleCommand(SDK::UWorld::GetWorld(), SDK::FString(L"startsafezone"), nullptr);

}