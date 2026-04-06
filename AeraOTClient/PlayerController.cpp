#include "PlayerController.h"
#include "Inventory.h"
#include <random>
#include "Utils.h"
#include "Husk.h"

SDK::AFortPlayerController* SavedController = nullptr;

std::vector<SDK::FVector> StartSpots = { 
    SDK::FVector(10.3f, 29.2f, 3347.5f),
    SDK::FVector(15.8f, -4380.6f, 1138.3f),
    SDK::FVector(3678.3f, -1045.8f, 2973.6f),
    SDK::FVector(-4537.5f, -705.5f, 2992.3f),
    SDK::FVector(89.3f, 4637.2f, 2143.0f),
    SDK::FVector(3481.6f, -2919.3f, 1975.9f),
    SDK::FVector(-4166.0f, 4101.8f, 2106.2f),
    SDK::FVector(-4802.0f, -5683.4f, 1745.6f),
    SDK::FVector(-3898.8f, -5354.9f, 2679.8f),
    SDK::FVector(3583.4f, 1576.4f, 2982.4f),
}; //only for PVP Tower so far I know off

void PlayerControllerAera::TeleportPlayer(SDK::AFortPlayerController* CurrentController, SDK::FVector Location, SDK::FRotator Rotation)
{
    if (!CurrentController)
        return;

    std::cout << "TeleportPlayer player controller!\n";

    auto Pawn = CurrentController->Pawn;
    if (Pawn)
    {
        Pawn->K2_TeleportTo(Location, Rotation);
    }
}

SDK::AFortPlayerController* PlayerControllerAera::GetLocalPlayerController()
{
    SDK::TArray<SDK::AActor*> PlayerControllers;
    SDK::UWorld* World = SDK::UWorld::GetWorld();
    SDK::UGameplayStatics::GetAllActorsOfClass(World, SDK::AFortPlayerController::StaticClass(), &PlayerControllers);
    if (PlayerControllers.Num() > 0)
    {
        auto Controller = (SDK::AFortPlayerController*)PlayerControllers[0];
        if (Controller)
        {
            SavedController = Controller;
            return Controller;
        }
    }
}

void PlayerControllerAera::PlacePlayerAtStartSpot(SDK::AFortPlayerController* CurrentController)
{
    if (!CurrentController)
    {
        return;
    }

    std::cout << "PlacePlayerAtStartSpot player controller!\n";

    std::random_device rd;
    std::mt19937 gen(rd()); // random generator
    std::uniform_int_distribution<> dis(0, StartSpots.size() - 1);

    int Random = dis(gen);
    TeleportPlayer(CurrentController, StartSpots.at(Random), {}); 
}

void PlayerControllerAera::CleanPlayer(SDK::AFortPlayerController* CurrentController)
{

    if (!CurrentController)
    {
        return;
    }

    std::cout << "Cleaning Inventory of player controller!\n";

    InventoryAera::RemoveInventoryItemsOfType(CurrentController, EINVENTORY_ITEM_TYPE::WeaponRanged);
    InventoryAera::RemoveInventoryItemsOfType(CurrentController, EINVENTORY_ITEM_TYPE::GadgetItem);
    InventoryAera::RemoveInventoryItemsOfType(CurrentController, EINVENTORY_ITEM_TYPE::ResourceItem, true);
}

void ApplyDefaultLoadOut(SDK::AFortPlayerController* CurrentController)
{
    if (!CurrentController)
    {
        return;
    }

    static auto RangedGun = SDK::UObject::FindObject<SDK::UFortWeaponRangedItemDefinition>("FortWeaponRangedItemDefinition Shotgun_Tactical_Alt2_T10.Shotgun_Tactical_Alt2_T10");
    if (RangedGun)
    {
        std::cout << "Found RangedGun Item Definition!\n";
    }

    InventoryAera::AddInventoryItem(CurrentController, RangedGun, SDK::EFortQuickBars::Primary, 1, 1, 30);
    InventoryAera::AddInventoryItem(CurrentController, RangedGun, SDK::EFortQuickBars::Primary, 2, 1, 30);
    InventoryAera::AddInventoryItem(CurrentController, RangedGun, SDK::EFortQuickBars::Primary, 3, 1, 30);
    InventoryAera::AddInventoryItem(CurrentController, RangedGun, SDK::EFortQuickBars::Primary, 4, 1, 30);
    InventoryAera::AddInventoryItem(CurrentController, RangedGun, SDK::EFortQuickBars::Primary, 5, 1, 30);
}

void PlayerControllerAera::StartClientController()
{
    std::cout << "StartClientController player controller!\n";
    SDK::TArray<SDK::AActor*> PlayerControllers;
    SDK::UWorld* World = SDK::UWorld::GetWorld();
    SDK::UGameplayStatics::GetAllActorsOfClass(World, SDK::AFortPlayerController::StaticClass(), &PlayerControllers);
    if (PlayerControllers.Num() > 0)
    {
        for (int i = 0; i < PlayerControllers.Num(); i++)
        {
            auto Controller = (SDK::AFortPlayerController*)PlayerControllers[i];
            PlacePlayerAtStartSpot(Controller);
            CleanPlayer(Controller);
            ApplyDefaultLoadOut(Controller);

            //Init All Husk Classes
            FortUtils::InitHusks();
            HuskAera::InitHusks();

            SDK::UFortWorldItem* BuildingTool = InventoryAera::GetWeaponFromName(Controller, "FortBuildingItemDefinition BuildingItemData_Wall.BuildingItemData_Wall");
            if (BuildingTool)
            {
                Controller->ServerExecuteInventoryItem(BuildingTool->GetItemGuid());
            }
            else
            {
                std::cout << "No Build Item To Execute!\n";
            }

            auto QuickBars = Controller->QuickBars;
            QuickBars->ForceNetUpdate();

            auto Pawn = Controller->MyFortPawn;
            Pawn->OnWeaponEquipped(Pawn->CurrentWeapon, Pawn->CurrentWeapon);

        }
    }
}

void DoSomthing()
{

}