#include "ProcessEventHooked.h"
#include "Building.h"
#include "Husk.h"
#include "PlayerController.h"
#include "Inventory.h"
#include "Bot.h"
#include <chrono>
#include <random>

bool PIsWorldReady = false;
ProcessEvent_t ProcessEventOriginal = nullptr;

float CurrentJumpTime = 3.f;
float MaxJumpCoolDownTime = 2.f;

std::chrono::high_resolution_clock::time_point LastJumpTime =
std::chrono::high_resolution_clock::now();

bool bReadyToSpawnHorde = false;

void* __fastcall ProcessEventBasic::ProcessEventHook(SDK::UObject* _Object, SDK::UObject* Object, SDK::UFunction* Function, void* Params)
{

   // std::cout << "Function: " << Function->GetFullName() << " Got Called From The ProcessEventHook!\n"; works
    std::string FulLFunctionName = Function->GetFullName();

    



    if (FulLFunctionName.contains("OnDamageServer"))
    {
        BuildingAera::OnDamageServer((SDK::Params::BuildingActor_OnDamageServer*)Params);

        std::cout << "OnDamageServer Got Called From ProcessEventHook!\n";
    }

    if (FulLFunctionName.contains("ServerCreateBuildingActor"))
    {
        //Function FortniteGame.BuildingActor.OnBuildingActorInitialized Got Called From ProcessEventHook!
        ProcessEventOriginal(_Object, Object, Function, Params);
        
        auto NewParams = (SDK::Params::FortPlayerController_ServerCreateBuildingActor*)Params;
        BuildingAera::OnServerCreateBuildingActor(NewParams);
    }

    if (FulLFunctionName.contains("Flinger"))
    {
        
        //FortniteUI.FortHUDContext.ForwardOnWeaponEquipped
        //EquippedItemWidget.EquippedItemWidget_C
        //PlayerPawn_Generic.PlayerPawn_Generic_C.OnWeaponEquipped

        //FortWeapon.OnWeaponVisibilityChanged
        
        //std::cout << "Important Function Called: " << FulLFunctionName << std::endl;

        //SDK::UAbilitySystemComponent::ServerTryActivateAbility();
    }

    if (FulLFunctionName.contains("HuskPawn.HuskPawn_C.ReceiveTick"))
    {
        auto NewParams = (SDK::Params::HuskPawn_C_ReceiveTick*)Params;
        HuskAera::HuskTick(_Object, NewParams->DeltaSeconds);
    }

    if (FulLFunctionName.contains("FlingerVimBlueprint"))
    {
        HuskAera::FlingerTick(_Object);
    }

    if (FulLFunctionName.contains("GameplayAbilities.GameplayAbility.K2_CommitExecute")) //preventing infinite double jumping
    {
        auto Spec = (SDK::UGameplayAbility*)_Object;
        auto AbilityFullName = Spec->GetFullName();
        if (AbilityFullName.contains("_Jump"))
        {
            auto PlayerController = PlayerControllerAera::GetLocalPlayerController();
            auto Character = PlayerController->Character;
            SDK::UKismetSystemLibrary::K2_SetTimer(Character, L"StopJumping", 0.4f, false);
        }
    }

    if (FulLFunctionName.contains("DeathWidget.DeathWidget_C.OnRespawnTimerUpdateAAA"))
    {
        //so we need to spawn a pawn again damn
        auto PlayerController = PlayerControllerAera::GetLocalPlayerController();
        if (!PlayerController->Pawn || !PlayerController->AcknowledgedPawn)
        {
            const SDK::FVector DefaultLocation = SDK::FVector(3583.4f, 1576.4f, 2982.4f);
            auto Pawn = FortUtils::SpawnActor<SDK::AFortPawn>(SDK::AFortPawn::StaticClass(), DefaultLocation, {});
            PlayerController->Possess(Pawn);
        }
    }

    if (FulLFunctionName.contains("ExecuteConsoleCommand")) //add own commands
    {
        SDK::Params::KismetSystemLibrary_ExecuteConsoleCommand* NewParams = (SDK::Params::KismetSystemLibrary_ExecuteConsoleCommand*)Params;
        std::string CommandStr = NewParams->Command.ToString();

        if (CommandStr == "Reload Player")
        {
            std::cout << "Reloading Player!\n";
            auto PlayerController = PlayerControllerAera::GetLocalPlayerController();
            PlayerController->bBlockInput = false;
            PlayerController->MyFortPawn->bIsDying = false;
            PlayerController->MyFortPawn->bIsHiddenForDeath = false;
        }
        else if (CommandStr == "Spawn Horde")
        {
            bReadyToSpawnHorde = true;
        }
        else if (CommandStr == "Spawn Demo Bot")
        {
            const SDK::FVector DefaultLocation = SDK::FVector(3581.4f, 1576.4f, 2982.4f);
            BotAera::SpawnBot(DefaultLocation);
        }
    }

    /*
    Important Function Called: Function DeathWidget.DeathWidget_C.OnPawnDied
Important Function Called: Function FortniteGame.FortPlayerControllerZone.ClientOnPawnDied
Important Function Called: Function FortniteGame.FortPawn.OnDeathServer
Important Function Called: Function GAB_GenericDeath.GAB_GenericDeath_C.K2_ActivateAbilityFromEvent
Important Function Called: Function FortniteGame.FortPawn.GameplayCue_InstantDeath
Important Function Called: Function PlayerPawn_Generic.PlayerPawn_Generic_C.OnDeathPlayEffects
Important Function Called: Function AnimNotify_HideBodyOnDeath.AnimNotify_HideBodyOnDeath_C.Received_Notify
    */

    if (FulLFunctionName.contains("DeathWidget.DeathWidget_C.OnPawnDied"))
    {
        return nullptr; //dont load lol
    }
    
    if (FulLFunctionName.contains("FortPlayerControllerZone.ClientOnPawnDied"))
    {
        auto Controller = PlayerControllerAera::GetLocalPlayerController();
        PlayerControllerAera::PlacePlayerAtStartSpot(Controller); //KSet Timer is better holy dawn
        Controller->MyFortPawn->ReceiveBeginPlay();
        return nullptr;
    }

    if (FulLFunctionName.contains("FortPawn.OnDeathServer"))
    {
        return nullptr;
    }

    /*
    Important Function Called: Function FortniteGame.FortPawn.OnDeathServer
Important Function Called: Function GAB_GenericDeath.GAB_GenericDeath_C.K2_ActivateAbilityFromEvent
Important Function Called: Function FortniteGame.FortPawn.GameplayCue_InstantDeath
Important Function Called: Function PlayerPawn_Generic.PlayerPawn_Generic_C.OnDeathPlayEffects
Important Function Called: Function AnimNotify_HideBodyOnDeath.AnimNotify_HideBodyOnDeath_C.Received_Notify
Important Function Called: Function DeathWidget.DeathWidget_C.OnPawnSpawned
    */

    if (FulLFunctionName.contains("GAB_GenericDeath.GAB_GenericDeath_C.K2_ActivateAbilityFromEvent"))
    {
        return nullptr;
    }

    if (FulLFunctionName.contains("AnimNotify_HideBodyOnDeath.AnimNotify_HideBodyOnDeath_C.Received_Notify"))
    {
        return nullptr;
    }

    if (FulLFunctionName.contains("FortniteGame.FortPawn.GameplayCue_InstantDeath"))
    {
        return nullptr;
    }

    if (FulLFunctionName.contains("PlayerPawn_Generic.PlayerPawn_Generic_C.OnDeathPlayEffects"))
    {
        return nullptr;
    }

    if (FulLFunctionName.contains("ServerAttemptInteract"))
    {
        std::cout << FulLFunctionName << " called!\n";

        auto NewParams = (SDK::Params::FortPlayerController_ServerAttemptInteract*)Params;
        auto ReceivingActor = NewParams->ReceivingActor;

        if(!ReceivingActor || !ReceivingActor->IsA(SDK::ABuildingContainer::StaticClass()))
            return ProcessEventOriginal(_Object, Object, Function, Params);

        auto Controller = PlayerControllerAera::GetLocalPlayerController();

        std::cout << "Resetting Loot From BuildingContainer!\n";

        /*
        auto BuildingContainer = (SDK::ABuildingContainer*)ReceivingActor;
        SDK::AFortPickup* FortPickup = FortUtils::SpawnActor<SDK::AFortPickup>(SDK::AFortPickup::StaticClass(), ReceivingActor->K2_GetActorLocation(), ReceivingActor->K2_GetActorRotation());

        static auto ItemDef = SDK::UObject::FindObject<SDK::UFortWeaponRangedItemDefinition>("FortWeaponRangedItemDefinition Shotgun_Tactical_Alt2_T10.Shotgun_Tactical_Alt2_T10");


        SDK::FFortItemEntry ItemEntry;
        ItemEntry.Count = 1;
        ItemEntry.LoadedAmmo = 200;
        ItemEntry.ItemDefinition = ItemDef;
        

        SDK::FFortItemEntry* PrimaryPickupItemEntry = &FortPickup->PrimaryPickupItemEntry;
        PrimaryPickupItemEntry->ItemDefinition = ItemEntry.ItemDefinition;
        PrimaryPickupItemEntry->Count = ItemEntry.Count;
        PrimaryPickupItemEntry->LoadedAmmo = ItemEntry.LoadedAmmo;
        FortPickup->OnRep_PrimaryPickupItemEntry();
        FortPickup->TossPickup(ReceivingActor->K2_GetActorLocation(), (SDK::AFortPawn*)Controller->Pawn, 1);
        */
    }

    if (bReadyToSpawnHorde)
    {
        bReadyToSpawnHorde = false;
        const SDK::FVector DefaultLocation(7021.1f, -77.7f, 219.5f);
        HuskAera::SpawnTestHorde(DefaultLocation, 20);
    }


    // Call original
    return ProcessEventOriginal(_Object, Object, Function, Params);
}
