#include "Inventory.h"


SDK::UClass* InventoryAera::GetItemDefinitionStaticClassOfType(EINVENTORY_ITEM_TYPE Type)
{
    switch (Type)
    {
    case WeaponMelee:
        return SDK::UFortWeaponMeleeItemDefinition::StaticClass();
    case WeaponRanged:
        return SDK::UFortWeaponRangedItemDefinition::StaticClass();
    case ResourceItem:
        return SDK::UFortResourceItemDefinition::StaticClass();
    case TrapItem:
        return SDK::UFortTrapItemDefinition::StaticClass();
    case GadgetItem:
        return SDK::UFortGadgetItemDefinition::StaticClass();
    case EditTool:
        return SDK::UFortEditToolItemDefinition::StaticClass();
    case BuildingItem:
        return SDK::UFortBuildingItemDefinition::StaticClass();
    case ConsumableItem:
        return SDK::UFortConsumableItemDefinition::StaticClass();
    case IngredientItem:
        return SDK::UFortIngredientItemDefinition::StaticClass();
    case AmmoItem:
        return SDK::UFortAmmoItemDefinition::StaticClass();
    default:
        return nullptr;
    }
}

void InventoryAera::RemoveInventoryItemsOfType(SDK::AFortPlayerController* Controller, EINVENTORY_ITEM_TYPE ItemType, bool CountToZero)
{
    if (!Controller || !Controller->WorldInventory)
        return;

    SDK::UClass* ItemDefTypeClass = GetItemDefinitionStaticClassOfType(ItemType);
    if (!ItemDefTypeClass)
        return;

    auto WorldInventory = Controller->WorldInventory;
    for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
    {
        auto CurrentItemInstance = WorldInventory->Inventory.ItemInstances[i];
        auto ItemDef = CurrentItemInstance->ItemEntry.ItemDefinition;

        if (ItemDef->GetFullName() == "None" || !ItemDef)
            continue;

        if (ItemDef->IsA(ItemDefTypeClass))
        {
            if (CountToZero)
            {
                CurrentItemInstance->ItemEntry.Count = 0;
                CurrentItemInstance->ItemEntry.bIsDirty = true;
                CurrentItemInstance->ItemEntry.ReplicationKey++;
            }
            else
            {
                std::cout << "Removing Item With ItemDefinition: " << ItemDef->GetFullName() << std::endl;
                CurrentItemInstance->ItemEntry.Count = 0;
                CurrentItemInstance->ItemEntry.bInStorageVault = false;
                CurrentItemInstance->ItemEntry.bIsDirty = true;
                CurrentItemInstance->ItemEntry.ItemDefinition = nullptr;
                CurrentItemInstance->ItemEntry.ReplicationKey++;
            }
        }
    }
}

SDK::UFortWorldItem* InventoryAera::GetWeaponFromName(SDK::AFortPlayerController* Controller, const std::string& WeaponName)
{
    if (!Controller || !Controller->WorldInventory)
        return nullptr;

    auto WorldInventory = Controller->WorldInventory;
    for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
    {
        if (WorldInventory->Inventory.ItemInstances[i]->ItemEntry.ItemDefinition->GetFullName() == WeaponName)
        {
            return WorldInventory->Inventory.ItemInstances[i];
        }
    }
    return nullptr;
}

bool GuidEquals(const SDK::FGuid& Lhs, const SDK::FGuid& Rhs)
{
    return
        Lhs.A == Rhs.A &&
        Lhs.B == Rhs.B &&
        Lhs.C == Rhs.C &&
        Lhs.D == Rhs.D;
}

SDK::UFortWorldItem* InventoryAera::FindItemByGuid(SDK::AFortPlayerController* Controller, SDK::FGuid Guid)
{
    if (!Controller || !Controller->WorldInventory)
        return nullptr;

    auto WorldInventory = Controller->WorldInventory;
    for (int i = 0; i < WorldInventory->Inventory.ItemInstances.Num(); i++)
    {
        if (GuidEquals(WorldInventory->Inventory.ItemInstances[i]->GetItemGuid(), Guid))
        {
            return WorldInventory->Inventory.ItemInstances[i];
        }
    }
    return nullptr;
}

int FindOpenSlot(SDK::AFortInventory* Inventory)
{
    if (!Inventory)
        return -1;

    auto ItemList = Inventory->Inventory;

    for (int i = 0; i < ItemList.ItemInstances.Num(); i++)
    {
        SDK::FFortItemEntry* CurrentEntry = &ItemList.ItemInstances[i]->ItemEntry;
        if (CurrentEntry == nullptr || CurrentEntry->ItemDefinition == nullptr)
        {
            return i;
        }
    }
    return -1;
}

void InventoryAera::AddInventoryItem(SDK::AFortPlayerController* Controller, SDK::UFortItemDefinition* ItemDef, SDK::EFortQuickBars Quickbar, uint32_t slot, uint32_t count, uint32_t LoadedAmmo)
{
    if (!Controller || !Controller->WorldInventory || !ItemDef)
        return;

    auto Item = ItemDef->CreateTemporaryItemInstanceBP();
    auto WorldItem = reinterpret_cast<SDK::UFortWorldItem*>(Item);
    WorldItem->ItemEntry.Count = 20;
    WorldItem->ItemEntry.Level = 60;
    WorldItem->ItemEntry.LoadedAmmo = Item->GetLoadedAmmo();
    WorldItem->ItemEntry.Durability = 300;
    Controller->WorldInventory->Inventory.ReplicatedEntries.Add(WorldItem->ItemEntry);
    Controller->WorldInventory->Inventory.ItemInstances.Add(WorldItem);
    Controller->QuickBars->ServerAddItemInternal(WorldItem->GetItemGuid(), Quickbar, slot);
    //and then send a inventory update somehow
}

