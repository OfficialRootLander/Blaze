#pragma once

#include "SDK/FortniteGame_classes.hpp"

typedef enum
{
	WeaponMelee, //Pickaxe
	WeaponRanged,
	ResourceItem,
	TrapItem,
	GadgetItem,
	EditTool,
	BuildingItem,
	ConsumableItem,
	IngredientItem,
	AmmoItem,
} EINVENTORY_ITEM_TYPE;

class InventoryAera
{
public:
	static SDK::UClass* GetItemDefinitionStaticClassOfType(EINVENTORY_ITEM_TYPE Type);
	static void RemoveInventoryItemsOfType(SDK::AFortPlayerController* Controller, EINVENTORY_ITEM_TYPE ItemType, bool CountToZero = false);
	static SDK::UFortWorldItem* GetWeaponFromName(SDK::AFortPlayerController* Controller, const std::string& WeaponName);
	static SDK::UFortWorldItem* FindItemByGuid(SDK::AFortPlayerController* Controller, SDK::FGuid Guid);
	static void AddInventoryItem(SDK::AFortPlayerController* Controller, SDK::UFortItemDefinition* ItemDef, SDK::EFortQuickBars Quickbar, uint32_t slot, uint32_t count, uint32_t LoadedAmmo);
};