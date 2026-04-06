#include "Building.h"
#include "Inventory.h"
#include "PlayerController.h"

int GetGiftedResourceCount()
{
    //curve
    return 5;
}

SDK::TArray<SDK::AActor*> BuildingAera::GetAllBuildingActors()
{
	SDK::TArray<SDK::AActor*> BuildingActors;
	SDK::UWorld* World = SDK::UWorld::GetWorld();
	SDK::UGameplayStatics::GetAllActorsOfClass(World, SDK::ABuildingActor::StaticClass(), &BuildingActors);
	return BuildingActors;
}

std::string GetResourceItemName(SDK::ABuildingSMActor* BuildingActor)
{
	if (BuildingActor->ResourceType == SDK::EFortResourceType::Wood)
	{
		return "FortResourceItemDefinition WoodItemData.WoodItemData";
	}
	else if (BuildingActor->ResourceType == SDK::EFortResourceType::Stone)
	{
		return "FortResourceItemDefinition StoneItemData.StoneItemData";
	}
	else if (BuildingActor->ResourceType == SDK::EFortResourceType::Metal)
	{
		return "FortResourceItemDefinition MetalItemData.MetalItemData";
	}
	return "FortResourceItemDefinition WoodItemData.WoodItemData";
}

std::string GetResourceNameBuildingActorClass(SDK::UClass* BuildingBluePrintClass)
{
	if(BuildingBluePrintClass->GetFullName().contains("W1"))
		return "FortResourceItemDefinition WoodItemData.WoodItemData";
	else if (BuildingBluePrintClass->GetFullName().contains("S1"))
		return "FortResourceItemDefinition StoneItemData.StoneItemData";
	else if (BuildingBluePrintClass->GetFullName().contains("M1"))
		return "FortResourceItemDefinition MetalItemData.MetalItemData";
	return "FortResourceItemDefinition WoodItemData.WoodItemData";
}

void BuildingAera::OnDamageServer(SDK::Params::BuildingActor_OnDamageServer* NewParams)
{
    SDK::FHitResult HitResult = NewParams->HitInfo;
    auto BuildingActor = (SDK::ABuildingSMActor*)HitResult.Actor.Get();
	uint8_t ResourceType = (uint8_t)BuildingActor->ResourceType;
    bool bHitWeakPoint = NewParams->Damage == 135;

    std::cout << "BuildingActor ResourceType: " << std::to_string(ResourceType) << "\n";

	if (!BuildingActor->IsA(SDK::ABuildingSMActor::StaticClass()))
		return;

	if (!NewParams->InstigatedBy->IsA(SDK::AFortPlayerController::StaticClass()))
		return;

    SDK::AFortPlayerController* Controller = (SDK::AFortPlayerController*)NewParams->InstigatedBy;
	if (!Controller)
	{
		std::cout << "No PlayerController Found!\n";
		return;
	}

	std::string ResourceItemname = GetResourceItemName(BuildingActor);

	uint8_t ResourceCount = GetGiftedResourceCount();
	int AmountOfResources = ResourceCount * (bHitWeakPoint + 1);

	//here actually give the item
	SDK::UFortWorldItem* Item = InventoryAera::GetWeaponFromName(Controller, ResourceItemname); //idk why called weapon but ok

	if (Item)
	{
		if (Item->ItemEntry.Count + AmountOfResources >= 1000)
			return;

		Item->ItemEntry.Count += ResourceCount;
		Item->ItemEntry.bIsDirty = true;
		Item->ItemEntry.ReplicationKey++;
	}
	else
	{
		std::cout << "Resource Item Could not be found in inventory!\n";
	}

}

void BuildingAera::OnServerCreateBuildingActor(SDK::Params::FortPlayerController_ServerCreateBuildingActor* NewParams)
{
	SDK::UClass* BuildingActorClass = NewParams->BuildingClassData.BuildingClass.Get();
	std::string ResourceName = GetResourceNameBuildingActorClass(BuildingActorClass);

	auto Controller = PlayerControllerAera::GetLocalPlayerController();
	if (!Controller)
		return;

	SDK::UFortWorldItem* Item = InventoryAera::GetWeaponFromName(Controller, ResourceName);

	if (!Item)
		return;

	if (Item->ItemEntry.Count - 10 < 0)
		return;

	Item->ItemEntry.Count -= 10;
	Item->ItemEntry.bIsDirty = true;
	Item->ItemEntry.ReplicationKey++;

	std::cout << "BuildingActorClass: " << BuildingActorClass->GetFullName() << "\n";
}