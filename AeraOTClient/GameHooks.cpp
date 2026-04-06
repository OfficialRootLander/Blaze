#include "GameHooks.h"
#include "ProcessEventHooked.h"

void GameHooks::InitAll()
{

    GMLHOOK::Init();

    auto SDKImageBase = SDK::InSDKUtils::GetImageBase();
    auto ProcessEventAddress = reinterpret_cast<void*>(SDKImageBase + SDK::Offsets::ProcessEvent);
    ProcessEvent = decltype(ProcessEvent)(SDKImageBase + SDK::Offsets::ProcessEvent);

    void* _defaultvoid = nullptr;

    GMLHOOK::Hook(ProcessEventAddress, &ProcessEventBasic::ProcessEventHook, ProcessEventOriginal, "ProcessEvent");
}