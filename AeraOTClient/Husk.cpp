#include "Husk.h"
#include "PlayerController.h"
#include <random>

#include <unordered_map>
#include "Building.h"

std::unordered_map<SDK::AHuskPawn_C*, float> HuskTickTimers;
std::unordered_map<SDK::AHuskPawn_C*, SDK::FVector> HuskLastLocation;
std::unordered_map<SDK::AHuskPawn_C*, float> HuskAttackTimers;
const float AttackCooldown = 1.0f; // seconds between melee attacks



bool bInitedHusks = false;

SDK::UAnimMontage* MeleeMontage = nullptr;

int TotalTicks = 0;

void HuskAera::InitHusks()
{
    //AnimMontage /Game/Animation/Game/Enemies/Husk/Montages/HuskMeleeMontage.HuskMeleeMontage
    MeleeMontage = FortUtils::SearchUObject<SDK::UAnimMontage>(L"AnimMontage HuskMeleeMontage.HuskMeleeMontage");
    if (!MeleeMontage)
    {
        std::cout << "Failure, Husk Melee Montage not found!\n";
    }
	//first test if static findobject works
	//FortUtils::SearchUObject(L"/Game/Characters/Enemies/Husk/Blueprints/HuskPawn.HuskPawn_C");
    
	///Game/Characters/Enemies/Husk/Blueprints/HuskPawn.HuskPawn_C
}

SDK::UClass* HuskAera::GetHuskBluePrintFast(std::string FullBluePrintClassName) //requires init
{
    for (int i = 0; i < HuskClasses.size(); i++)
    {
        auto HuskClass = HuskClasses.at(i);
        if (!HuskClass) continue;

        if (HuskClass->GetFullName() == FullBluePrintClassName)
            return HuskClass;
    }
    return nullptr;
}

SDK::UClass* HuskAera::GetClassOfHuskType(HUSK_AERA_TYPE TYPE) //NO BEHIVE FOR NOW
{
    switch (TYPE)
    {
    case NORMAL: return SDK::AHuskPawn_C::StaticClass();
    case HUSK_BOMBSHELL: return SDK::AHuskPawn_Bombshell_C::StaticClass();
    case HUSK_BOMBSHELL_POISON: return SDK::AHuskPawn_Bombshell_Poison_C::StaticClass();
    //case HUSK_BEEHIVE: return GetHuskBluePrintFast("BlueprintGeneratedClass HuskPawn_Beehive.HuskPawn_Beehive_C");
    case HUSK_DWARF: return GetHuskBluePrintFast("BlueprintGeneratedClass HuskPawn_Dwarf.HuskPawn_Dwarf_C");
    case HUSK_DWARF_FIRE: return GetHuskBluePrintFast("BlueprintGeneratedClass HuskPawn_Dwarf_Fire.HuskPawn_Dwarf_Fire_C");
    case HUSK_DWARF_ICE: return GetHuskBluePrintFast("BlueprintGeneratedClass HuskPawn_Dwarf_Ice.HuskPawn_Dwarf_Ice_C");
    case HUSK_DWARF_LIGHTNING: return GetHuskBluePrintFast("BlueprintGeneratedClass HuskPawn_Dwarf_Lightning.HuskPawn_Dwarf_Lightning_C");
    case HUSK_FIRE: return GetHuskBluePrintFast("BlueprintGeneratedClass HuskPawn_Fire.HuskPawn_Fire_C");
    case HUSK_ICE: return GetHuskBluePrintFast("BlueprintGeneratedClass HuskPawn_Ice.HuskPawn_Ice_C");
    case HUSK_LIGHTNING: return GetHuskBluePrintFast("BlueprintGeneratedClass HuskPawn_Lightning.HuskPawn_Lightning_C");
    case HUSK_PITCHER: return GetHuskBluePrintFast("BlueprintGeneratedClass HuskPawn_Pitcher.HuskPawn_Pitcher_C");
	//case BLASTER: return GetHuskBluePrintFast("BlueprintGeneratedClass BlasterPawn.BlasterPawn_C");
    //case FLINGER: return GetHuskBluePrintFast("BlueprintGeneratedClass FlingerPawn.FlingerPawn_C");
    case FLINGER: return SDK::AFlingerPawn_C::StaticClass();
    case HALLOWEEN_WEREWOLF: return GetHuskBluePrintFast("BlueprintGeneratedClass WerewolfPawn.WerewolfPawn_C");
    default: return SDK::AHuskPawn_C::StaticClass();
    }
}

void HuskAera::SpawnHusk(HUSK_AERA_TYPE HuskType, SDK::FVector Location, SDK::FRotator Rotation)
{

	auto Class = GetClassOfHuskType(HuskType);  
    if (!Class)
    {
        std::cout << "Failed to find class for Husk Type: " << static_cast<int>(HuskType) << "\n";
		return;
    }

    auto Husk = FortUtils::SpawnActor<SDK::AHuskPawn_C>(
        Class,
        Location,
        Rotation
    );

    auto NewAIController = FortUtils::SpawnActor<SDK::AAIController>(
        SDK::AAIController::StaticClass(),
        Location,
        Rotation
    );

    NewAIController->Possess(Husk);

    Husk->PrimaryActorTick.bCanEverTick = true;

    Husk->SetActorTickEnabled(true);
}

void HuskAera::SpawnTestHorde(SDK::FVector HordeLocation, int HusksToSpawn)
{
    float MaxSpread = 600.f;
    float MinimumSpread = 30.f;

    std::random_device rd;
    std::mt19937 gen(rd()); 
    std::uniform_real_distribution<float> dis(MinimumSpread, MaxSpread);
    std::uniform_int_distribution<int> sign(0, 1);
    std::uniform_int_distribution<int> huskDist(0, 11);

    for (int i = 0; i < HusksToSpawn; i++)
    {
        float XSpread = dis(gen) * (sign(gen) ? 1.f : -1.f);
        float YSpread = dis(gen) * (sign(gen) ? 1.f : -1.f);
        SDK::FVector NewSpawnVector = SDK::FVector(HordeLocation.X + XSpread, HordeLocation.Y + YSpread, HordeLocation.Z);

        HUSK_AERA_TYPE TypeToSpawn = static_cast<HUSK_AERA_TYPE>(huskDist(gen));

        HuskAera::SpawnHusk(TypeToSpawn, NewSpawnVector, {});
    }
}

SDK::FVector NormalizeSafe(SDK::FVector& Dir, int Speed)
{
    float Mag = sqrtf(Dir.X * Dir.X + Dir.Y * Dir.Y);
    if (Mag > 0.f)
    {
        Dir.X = Dir.X / Mag * Speed;
        Dir.Y = Dir.Y / Mag * Speed;
        Dir.Z = Dir.Z / Mag * Speed;
    }
    else
    {
        Dir.X = 0.f;
        Dir.Y = 0.f;
    }
    return Dir;
}

void HuskAttack(void* _Object, SDK::AActor* Actor, bool IsPlayer)
{
    auto Husk = (SDK::AHuskPawn_C*)_Object; //can be other husks
    Husk->TakerAttackFSM(true);

    if (IsPlayer)
    {
        auto Controller = (SDK::AFortPlayerController*)Actor;
        auto HealthSet = Controller->MyFortPawn->HealthSet;

        if (HealthSet->CurrentShield <= 0)
        {
            auto Pawn = Controller->MyFortPawn;
            Pawn->SetHealth(HealthSet->Health -= 10.f);
            std::cout << "Setting Health From Husk Damage!\n";
        }
        else
        {
            HealthSet->CurrentShield -= 10.f;
        }
    }

    //handle building actors
}

void HuskAttemptDamageBuildingActor(void* _Object)
{
    auto Husk = (SDK::AHuskPawn_C*)_Object; //can be other husks
    SDK::FVector HuskLoc = Husk->K2_GetActorLocation();
    SDK::TArray<SDK::AActor*> BuildingActors = BuildingAera::GetAllBuildingActors();
    float ClosestDistSq = FLT_MAX;
    SDK::ABuildingActor* ClosestBuilding = nullptr;
    for (int i = 0; i < BuildingActors.Num(); i++)
    {
        auto CurrentBuildingActor = BuildingActors[i];
        if (!CurrentBuildingActor)
            continue;

        SDK::FVector BuildingLoc = CurrentBuildingActor->K2_GetActorLocation();
        float DistSq = (BuildingLoc - HuskLoc).X * (BuildingLoc - HuskLoc).X +
            (BuildingLoc - HuskLoc).Y * (BuildingLoc - HuskLoc).Y +
            (BuildingLoc - HuskLoc).Z * (BuildingLoc - HuskLoc).Z;

        auto BuildingActor = (SDK::ABuildingActor*)CurrentBuildingActor;
        if (DistSq < ClosestDistSq )
        {
            ClosestDistSq = DistSq;
            ClosestBuilding = BuildingActor;
        }
    }

    if (ClosestBuilding)
    {
        ClosestBuilding->K2_DestroyActor();
    }
}

void HuskAera::HuskTick(void* _Object, float DeltaTime)
{
    auto Controller = SavedController;
    if (!Controller)
        return;

    auto Husk = (SDK::AHuskPawn_C*)_Object;

    if (Husk->Health <= 0 || Husk->HealthSet->Health <= 0)
    {
        Husk->K2_DestroyActor();
        return;
    }

    HuskTickTimers[Husk] += DeltaTime;
    HuskAttackTimers[Husk] += DeltaTime;

    if (HuskTickTimers[Husk] < 0.03f)
        return;

    HuskTickTimers[Husk] = 0.f;

    auto ControllerLoc = Controller->MyFortPawn->K2_GetActorLocation();
    auto HuskLoc = Husk->K2_GetActorLocation();
    auto HuskRot = Husk->K2_GetActorRotation();

    SDK::FVector Dir = ControllerLoc - HuskLoc;
    float Speed = 20.f;
    NormalizeSafe(Dir, Speed);

    float Yaw = atan2f(Dir.Y, Dir.X) * (180.f / 3.14159265f);
    SDK::FRotator LookAtRot;
    LookAtRot.Pitch = HuskRot.Pitch;
    LookAtRot.Roll = HuskRot.Roll;
    LookAtRot.Yaw = Yaw;
    Husk->SetActorRotation(LookAtRot);

    auto NewDir = (ControllerLoc - HuskLoc);
    float DistanceToPlayer = sqrtf(NewDir.X * NewDir.X + NewDir.Y * NewDir.Y + NewDir.Z * NewDir.Z);

    bool bIsFalling = Husk->CharacterMovement->MovementMode == SDK::EMovementMode::MOVE_Falling;

    if (!bIsFalling && DistanceToPlayer < 4000.f)
    {
        if (Husk->bIsSleeping)
        {
            Husk->bHasEngaged = true;
            Husk->bIsSleeping = false;
            Husk->OnRep_bIsSleeping();
        }
        Husk->CharacterMovement->Velocity = Dir * Speed;

        if (HuskLastLocation.find(Husk) != HuskLastLocation.end())
        {
            SDK::FVector CurrentLoc = Husk->K2_GetActorLocation();
            SDK::FVector LastLoc = HuskLastLocation[Husk]; // if first tick, will default to zero vector
            SDK::FVector DeltaMove = CurrentLoc - LastLoc;
            float DistMoved = sqrtf(DeltaMove.X * DeltaMove.X + DeltaMove.Y * DeltaMove.Y + DeltaMove.Z * DeltaMove.Z);

            if (DistMoved < 10.f) 
            {
                // do your logic here
                //HuskAttemptDamageBuildingActor(Husk); //this is gonna lag like shit but ok!

            }
            //std::cout << "Distance Moved: " << std::to_string(DistMoved) << "\n";
        }
    }
    else
    {
        if (!bIsFalling && !Husk->bIsSleeping)
        {
            Husk->bIsSleeping = true;
            Husk->OnRep_bIsSleeping();
        }
    }

    if (DistanceToPlayer < 250.f)
    {
        if (HuskAttackTimers[Husk] >= AttackCooldown)
        {
            HuskAttack(Husk, Controller, true); // apply damage

            // Play animation
            if (MeleeMontage)
                Husk->PlayAnimMontage(MeleeMontage, 1.0f, SDK::FName());

            HuskAttackTimers[Husk] = 0.f; // reset timer
        }       
    }

    HuskLastLocation[Husk] = HuskLoc;
}

void HuskAera::FlingerTick(void* Object) //UFlingerVimBlueprint_New_C function
{
    auto Flinger = (SDK::AFlingerPawn_C*)Object;
    if (!Flinger)
        return;

    auto Controller = SavedController;
    if (!Controller)
        return;

    SDK::TArray<SDK::AActor*> FlingerActors;
    SDK::UGameplayStatics::GetAllActorsOfClass(SDK::UWorld::GetWorld(), SDK::AFlingerPawn_C::StaticClass(), &FlingerActors);

    for (int i = 0; i < FlingerActors.Num(); i++)
    {

        auto FlingerPawn = (SDK::AFlingerPawn_C*)FlingerActors[i];
        if (!FlingerPawn) continue;

        //FlingerPawn->FlingerSpawnedAnEnemy()
        //FlingerPawn->BeginTakerSwoopAttack();
        // 
        //FlingerPawn->
        

        /*
        //FlingerPawn->K2_TeleportTo(SavedController->MyFortPawn->K2_GetActorLocation(), {});
        auto ControllerLoc = Controller->MyFortPawn->K2_GetActorLocation();
        auto HuskLoc = Husk->K2_GetActorLocation();
        auto HuskRot = Husk->K2_GetActorRotation();

        SDK::FVector Dir = ControllerLoc - HuskLoc;
        float Speed = 20.f;
        NormalizeSafe(Dir, Speed);

        float Yaw = atan2f(Dir.Y, Dir.X) * (180.f / 3.14159265f);
        SDK::FRotator LookAtRot;
        LookAtRot.Pitch = HuskRot.Pitch;
        LookAtRot.Roll = HuskRot.Roll;
        LookAtRot.Yaw = Yaw;
        Husk->SetActorRotation(LookAtRot);

        auto NewDir = (ControllerLoc - HuskLoc);
        float DistanceToPlayer = sqrtf(NewDir.X * NewDir.X + NewDir.Y * NewDir.Y + NewDir.Z * NewDir.Z);

        bool bIsFalling = Husk->CharacterMovement->MovementMode == SDK::EMovementMode::MOVE_Falling;

        if (!bIsFalling && DistanceToPlayer < 4000.f)
        {
            Husk->CharacterMovement->Velocity = Dir * Speed;

        }
        else
        {

        }
        */
        
        
        
        
    }
    
    //std::cout << "Flinger FullName: " << Flinger->GetFullName() << "\n";
}