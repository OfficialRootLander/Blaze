#pragma once
#ifndef WINDOW_H
#define WINDOW_H


#include <windows.h>
#include <d3d11.h>
#include <tchar.h>
#include "imgui.h"
#include "backends/imgui_impl_win32.h"
#include "backends/imgui_impl_dx11.h"

#pragma comment(lib, "d3d11.lib")

#include "SDK/FortniteGame_classes.hpp"
#include "PlayerController.h"
#include "Inventory.h"
#include "SDK/FortniteEntry_classes.hpp"
#include "Husk.h"
#include "ProcessEventHooked.h"
#include "Storm.h"
#include "Match.h"
#include "ConnectSpawner.h"
#include <random>
#include "Bot.h"
//#include "PlayerController.h"
//#include "Inventory.h"

// Globals
static ID3D11Device* g_pd3dDevice = NULL;
static ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
static IDXGISwapChain* g_pSwapChain = NULL;
static ID3D11RenderTargetView* g_mainRenderTargetView = NULL;
static HWND g_hWnd = NULL;

/*
struct YYValue {
    double value;
    uint32_t flags;
    uint32_t type;
};

void Call_sub_53BA30_example()
{
    YYValue roomVal{};
    roomVal.value = 68; // room ID
    roomVal.flags = 0;
    roomVal.type = 0;  // number

    // Call the proper function that actually changes rooms
    ((int(__cdecl*)(int, int, int, int, int))0x4F3140)(
        0, 0, 0, 0,
        (int)&roomVal
        );
}
*/

char CommandBuffer[256];

int ObjectSpawnCounter = 1;
inline int HuskCounter = 0;

// Forward declare message handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message loop handler
inline LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            ID3D11Texture2D* pBackBuffer;
            g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
            pBackBuffer->Release();
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, msg, wParam, lParam);
}

// Create D3D11 device and swap chain
inline bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[1] = { D3D_FEATURE_LEVEL_11_0 };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags,
        featureLevelArray, 1, D3D11_SDK_VERSION, &sd, &g_pSwapChain,
        &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
    return true;
}

inline void CleanupDeviceD3D()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

// Main window + ImGui loop
inline int RunImGuiWindow(HINSTANCE hInstance)
{
    // Register window class
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L,
        GetModuleHandle(NULL), NULL, NULL, NULL, NULL,
        _T("ImGui Window Class"), NULL };
    RegisterClassEx(&wc);
    g_hWnd = CreateWindow(wc.lpszClassName, _T("Blaze"),
        WS_OVERLAPPEDWINDOW, 100, 100, 450, 400,
        NULL, NULL, wc.hInstance, NULL);

    // Initialize D3D
    if (!CreateDeviceD3D(g_hWnd))
    {
        CleanupDeviceD3D();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ShowWindow(g_hWnd, SW_SHOWDEFAULT);
    UpdateWindow(g_hWnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplWin32_Init(g_hWnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Main loop
    MSG msg;
    ZeroMemory(&msg, sizeof(msg));
    while (msg.message != WM_QUIT)
    {
        if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            continue;
        }

        // Start new frame
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        // Example window with slider
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowBgAlpha(0.5f); // Optional: semi-transparent background
        ImGui::Begin("HUD", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoCollapse);

        int roomSlot = 68;

        
        if (ImGui::Button("TP All PlayerControllers to Base"))
        {
            SDK::TArray<SDK::AActor*> PlayerControllers;
            SDK::UWorld* World = SDK::UWorld::GetWorld();
            SDK::UGameplayStatics::GetAllActorsOfClass(World, SDK::AFortPlayerController::StaticClass(), &PlayerControllers);
            for (int i = 0; i < PlayerControllers.Num(); i++)
            {
                auto CurrentController = (SDK::AFortPlayerController*)(PlayerControllers[i]);
                if (CurrentController)
                {
                    auto Pawn = CurrentController->Pawn;
                    if (Pawn)
                    {
                        const SDK::FVector Location = SDK::FVector(3583.4f, 1576.4f, 2982.4f); //for PVP Tower
                        Pawn->K2_TeleportTo(Location, {});
                        //CurrentController->ClientTravel(L"/Game/Maps/AITestbed_2", SDK::ETravelType::TRAVEL_MAX, true, )
                    }
                }
            }
        }

        if (ImGui::Button("Remove Inventory Weapons"))
        {
            auto CurrentController = PlayerControllerAera::GetLocalPlayerController();
            if (CurrentController)
            {
                InventoryAera::RemoveInventoryItemsOfType(CurrentController, EINVENTORY_ITEM_TYPE::WeaponRanged);
            }
        }

        if (ImGui::Button("Remove Inventory GadgetItems"))
        {
            auto CurrentController = PlayerControllerAera::GetLocalPlayerController();
            if (CurrentController)
            {
                InventoryAera::RemoveInventoryItemsOfType(CurrentController, EINVENTORY_ITEM_TYPE::GadgetItem);
            }
        }

        if (ImGui::Button("Remove Inventory Resources"))
        {
            auto CurrentController = PlayerControllerAera::GetLocalPlayerController();
            if (CurrentController)
            {
                InventoryAera::RemoveInventoryItemsOfType(CurrentController, EINVENTORY_ITEM_TYPE::ResourceItem, true);
            }
        }
        
        if (ImGui::Button("Weapon Test"))
        {
            auto CurrentController = PlayerControllerAera::GetLocalPlayerController();
        }

        if (ImGui::Button("Start Husk AI"))
        {
            SDK::FVector DefaultLocation = SDK::FVector(3581.4f, 1576.4f, 2982.4f);
            HuskAera::SpawnHusk(BLASTER, DefaultLocation, {});


            /*
            auto CurrentController = PlayerControllerAera::GetLocalPlayerController();
            CurrentController->CheatManager->Summon(L"HuskPawn_C");
            //CurrentController->CheatManager->Summon(L"HuskPawn_C");
            ///CurrentController->CheatManager->Summon(L"HuskPawn_C");
           // CurrentController->CheatManager->Summon(L"HuskPawn_C");


            std::cout << "Can be Damaged: " << std::to_string(CurrentController->MyFortPawn->bCanBeDamaged) << "\n";
            CurrentController->MyFortPawn->bCanBeDamaged = true;

            SDK::TArray<SDK::AActor*> Husks;
            SDK::UWorld* World = SDK::UWorld::GetWorld();
            SDK::UGameplayStatics::GetAllActorsOfClass(World, SDK::AHuskPawn_C::StaticClass(), &Husks);

            for (int i = 0; i < Husks.Num(); i++)
            {
                auto CurrentHusk = (SDK::AHuskPawn_C*)Husks[i];
                const SDK::FVector DefaultLocation = SDK::FVector(3583.4f, 1576.4f, 2982.4f);
                CurrentHusk->StartMovingAndThinking();
                CurrentHusk->K2_TeleportTo(DefaultLocation, {});
                CurrentHusk->OnRep_FinishSpawning();
                CurrentHusk->FinishSpawning();
                CurrentHusk->bIsCharging = true;
                //CurrentHusk->StartMovingAndThinking();
                //CurrentHusk->HuskEvadeStart();
            }
            */
  
        }

        if (ImGui::Button("Spawn Flinger"))
        {
            const SDK::FVector DefaultLocation = SDK::FVector(3581.4f, 1576.4f, 2982.4f);
            HuskAera::SpawnHusk(HUSK_AERA_TYPE::FLINGER, DefaultLocation, SDK::FRotator(0.f, 0.f, 0.f));
        }

        if (ImGui::Button("Reload Player After Respawn"))
        {
            SDK::UKismetSystemLibrary::ExecuteConsoleCommand(SDK::UWorld::GetWorld(), SDK::FString(L"Reload Player"), nullptr);
        }

        if (ImGui::Button("Spawn Horde"))
        {
            bReadyToSpawnHorde = true;
        }

        if (ImGui::Button("Start StormManager"))
        {
            StormAera::StartBasicStorm();

        }

        if (ImGui::Button("Start Mission"))
        {
            auto World = SDK::UWorld::GetWorld();
            auto GameMode = World->AuthorityGameMode;
            
        }
        
        if (ImGui::Button("Spawn Demo Controller"))
        {
            //const SDK::FVector DefaultLocation = SDK::FVector(3581.4f, 1576.4f, 2982.4f);
			//BotAera::SpawnBot(DefaultLocation, SDK::FRotator(0.f, 0.f, 0.f));   
            SDK::UKismetSystemLibrary::ExecuteConsoleCommand(SDK::UWorld::GetWorld(), SDK::FString(L"Spawn Demo Bot"), nullptr);
        }


        //here!!



        ImGui::End();

        // Render
        ImGui::Render();
        const float clear_color[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0); // Present with vsync
    }

    // Cleanup
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();
    CleanupDeviceD3D();
    DestroyWindow(g_hWnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
    return 0;
}

DWORD WINAPI RUNWINDOW(LPVOID Params)
{
    RunImGuiWindow(GetModuleHandleA(NULL));
    CreateThread(0, 0, 0, RunImGuiWindow, 0, 0);

    return 0;
}



#endif // WINDOW_H
