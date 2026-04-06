
#include "framework.h"
#include "PlayerController.h"
#include "Window.h"
#include "GameHooks.h"


DWORD WINAPI MainThread(LPVOID lParams)
{
    AllocConsole();
    FILE* f;


    freopen_s(&f, "CONIN$", "r", stdin);
    freopen_s(&f, "CONOUT$", "w", stderr);
    freopen_s(&f, "CONOUT$", "w", stdout);
    SetConsoleTitleA("AeraOTClient (FortniteWin32) UE 4.12");

    PlayerControllerAera::StartClientController();

    GameHooks::InitAll();

    CreateThread(0, 0, RUNWINDOW, 0, 0, 0);

    DoSomthing();

    return 0;
}


BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID lpReserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        HANDLE hThread = CreateThread(nullptr, 0, MainThread, hModule, 0, nullptr);
        if (!hThread) {
            MessageBoxA(NULL, "CreateThread failed!", "Error", MB_OK | MB_ICONERROR);
        }
        else {
            CloseHandle(hThread);
        }
    }
    return TRUE;
}

