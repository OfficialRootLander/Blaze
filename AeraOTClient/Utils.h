#pragma once
#include <windows.h>

#include "SDK/CoreUObject_classes.hpp"
#include "SDK/FortniteGame_classes.hpp"
#include "SDK/Engine_classes.hpp"

#define TCHAR wchar_t*

static std::string WStringToString(const wchar_t* wstr)
{
	if (!wstr)
		return std::string();

	int sizeNeeded = WideCharToMultiByte(
		CP_UTF8,
		0,
		wstr,
		-1,
		nullptr,
		0,
		nullptr,
		nullptr
	);

	std::string result(sizeNeeded - 1, 0);

	WideCharToMultiByte(
		CP_UTF8,
		0,
		wstr,
		-1,
		&result[0],
		sizeNeeded,
		nullptr,
		nullptr
	);

	return result;
}

inline bool bHusksInitialized = false;
inline std::vector<SDK::UClass*> HuskClasses;

class FortUtils
{
public:
	static inline SDK::UObject* (*StaticSearchUObjectOriginal)(SDK::UClass* Class, SDK::UObject* InOuter, const wchar_t* Name, bool ExactClass) = nullptr;

	//HuskPawn_Dwarf_Fire.uasset

	static inline void InitHusks()
	{
		if (bHusksInitialized)
			return;

		auto GObjects = reinterpret_cast<SDK::TArray<SDK::FUObjectItem>*>(
			SDK::InSDKUtils::GetImageBase() + SDK::Offsets::GObjects
			);

		if (!GObjects)
			return;

		int Count = GObjects->Num();

		for (int i = 0; i < Count; i++)
		{
			SDK::FUObjectItem& Item = (*GObjects)[i];
			SDK::UObject* Obj = Item.Object;

			if (!Obj)
				continue;

			// Faster check first: class type
			if (!Obj->IsA(SDK::UClass::StaticClass()))
				continue;

			std::string Name = Obj->GetFullName();

			if (!Obj->IsA(SDK::UBlueprintGeneratedClass::StaticClass()))
				continue;

			if (Name.contains("BlasterPawn_C"))
				goto skip;

			// Must contain HuskPawn
			if (!(Name.contains("HuskPawn") ||
				Name.contains("WerewolfPawn") ||
				Name.contains("SmasherPawn") ||
				Name.contains("TakerPawn") ||
				Name.contains("FlingerPawn")))
			{
				continue;
			}

			// Exclusions
			if (Name.contains("Projectile") ||
				Name.contains("GAB_") ||
				Name.contains("GE_") ||
				Name.contains("AnimBlueprint") ||
				Name.contains("Interface") ||
				Name.contains("Variant"))
				continue;

			skip:

			HuskClasses.push_back(static_cast<SDK::UClass*>(Obj));
			std::cout << "[HUSK INIT] Added: " << Name << "\n";
		}

		bHusksInitialized = true;

		std::cout << "[HUSK INIT] Total Husk Classes: "
			<< HuskClasses.size() << "\n";
	}

	template <typename T = SDK::UObject>
	static inline T* SearchUObject(const wchar_t* ObjectPath)
	{
		auto GObjects = reinterpret_cast<SDK::TArray<SDK::FUObjectItem>*>(
			SDK::InSDKUtils::GetImageBase() + SDK::Offsets::GObjects
			);

		if (!GObjects)
			return nullptr;

		std::string TargetName = WStringToString(ObjectPath);

		int Count = GObjects->Num();

		for (int i = 0; i < Count; i++)
		{
			SDK::FUObjectItem& Item = (*GObjects)[i];
			SDK::UObject* Obj = Item.Object;

			if (!Obj)
				continue;

			std::string FullName = Obj->GetFullName();

			if (FullName.contains("Pawn") && FullName.contains("BlueprintGeneratedClass") && FullName.contains("_C"))
			{
				std::cout << " Pawn ClassObject Found: " << FullName << "\n";
			}

			if (FullName == TargetName)
			{
				std::cout << "[+] FOUND MATCH: " << FullName << std::endl;
				return static_cast<T*>(Obj);
			}
		}

		return nullptr;
	}

	template <typename O>
	static O* StaticLoadObject(const wchar_t* ObjectPath, SDK::UClass* ObjectClass = nullptr)
	{
		O* NewPtr = nullptr;
		return (O*)NewPtr; //for now
	}

	static inline SDK::UWorld* GetUpdatedWorld()
	{
		static SDK::UEngine* Engine = FortUtils::SearchUObject<SDK::UEngine>(L"/Engine/Transient.FortEngine_0");
		if (Engine)
		{
			if (Engine->GameViewport)
			{
				return Engine->GameViewport->World;
			}
		}
		return nullptr;
	}

	template <typename O>
	static inline O* SpawnActorDefault(SDK::UClass* StaticClass, SDK::FVector Position = SDK::FVector(0, 0, 0), SDK::FRotator Rotation = SDK::FRotator(0, 0, 0))
	{
		//pls fix this like good

		SDK::FTransform transform;
		transform.Rotation = SDK::FQuat(0);
		transform.Scale3D = SDK::FVector(1, 1, 1);
		transform.Translation = Position;

		//TODO later add full calc system for every component

		static auto GamePlayStatsClass = (SDK::UGameplayStatics*)SDK::UGameplayStatics::StaticClass();
		SDK::AActor* NewActor = GamePlayStatsClass->BeginDeferredActorSpawnFromClass(SDK::UWorld::GetWorld(), StaticClass, transform, SDK::ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, nullptr);
		GamePlayStatsClass->FinishSpawningActor(NewActor, transform);
		return (O*)(NewActor);
	}

	//pls replace later
	template <typename ActorType>
	static ActorType* SpawnActor(SDK::UClass* Class, SDK::FVector Location, SDK::FRotator Rotation = SDK::FRotator(), SDK::FVector Scale3D = SDK::FVector(1, 1, 1))
	{
		SDK::FQuat Quat;
		SDK::FTransform Transform;

		auto DEG_TO_RAD = 3.14159 / 180;
		auto DIVIDE_BY_2 = DEG_TO_RAD / 2;

		auto SP = sin(Rotation.Pitch * DIVIDE_BY_2);
		auto CP = cos(Rotation.Pitch * DIVIDE_BY_2);

		auto SY = sin(Rotation.Yaw * DIVIDE_BY_2);
		auto CY = cos(Rotation.Yaw * DIVIDE_BY_2);

		auto SR = sin(Rotation.Roll * DIVIDE_BY_2);
		auto CR = cos(Rotation.Roll * DIVIDE_BY_2);

		Quat.X = CR * SP * SY - SR * CP * CY;
		Quat.Y = -CR * SP * CY - SR * CP * SY;
		Quat.Z = CR * CP * SY - SR * SP * CY;
		Quat.W = CR * CP * CY + SR * SP * SY;

		Transform.Rotation = Quat;
		Transform.Scale3D = Scale3D;
		Transform.Translation = Location;

		static auto GamePlayStatsClass = (SDK::UGameplayStatics*)SDK::UGameplayStatics::StaticClass();
		SDK::AActor* Actor = GamePlayStatsClass->BeginDeferredActorSpawnFromClass(SDK::UWorld::GetWorld(), Class, Transform, SDK::ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn, nullptr);
		GamePlayStatsClass->FinishSpawningActor(Actor, Transform);
		return (ActorType*)Actor;
	}

	static inline SDK::APawn* SpawnPlayerPawn(SDK::AFortPlayerController* Controller, SDK::FVector StartSpot, SDK::FRotator Rotation, bool NewController = true)
	{
		auto Pawn = FortUtils::SpawnActor<SDK::AFortPawn>(SDK::AFortPawn::StaticClass(), StartSpot, Rotation);
		SDK::UWorld* World = SDK::UWorld::GetWorld();


		Pawn->bCanBeDamaged = NewController ? false : true;
		Pawn->Owner = Controller;
		Pawn->OnRep_Owner();

		Controller->AcknowledgedPawn = Pawn;
		Controller->Pawn = Pawn;
		Controller->OnRep_Pawn();
		Controller->Possess(Pawn);


		Pawn->OnRep_ReplicatedMovement();


		Controller->bHasServerFinishedLoading = true;
		Controller->OnRep_bHasServerFinishedLoading();

		//Controller->AcknowledgedPawn = Pawn;

		//Controller->AcknowledgedPawn = Pawn;
		
		Pawn->SetHealth(100);
		auto HealthSet = Pawn->HealthSet;
		HealthSet->OnRep_Shield();
		HealthSet->OnRep_CurrentShield();


		if (NewController)
		{


			Controller->bIsDisconnecting = false;
			Controller->bHasClientFinishedLoading = true;
			Controller->bHasServerFinishedLoading = true;
			Controller->bHasInitiallySpawned = true;
		}
		else
		{

			//NewQuickBars->SetOwner(Controller);
			//Controller->QuickBars = NewQuickBars;
			//Controller->OnRep_QuickBar();
			//Controller->OnRep_QuickBar();
		}

		return Pawn;
	}



};