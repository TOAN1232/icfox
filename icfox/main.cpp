#include <windows.h>
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

#include "Memory.hpp"
#include "Renderer.hpp"
#include "Entities.hpp"
#include "ESP.hpp"
#include "Menu.hpp"
#include "Config.hpp"

// Global variable definitions for namespaces defined in headers
namespace Memory {
    HANDLE hProcess = NULL;
    DWORD processId = 0;
    HWND hwnd = NULL;
    uintptr_t baseAddress = 0;
}

namespace Entities {
    std::vector<PlayerEntity> players;
    PlayerEntity localPlayer;
    uintptr_t dataModel = 0;
    uintptr_t visualEngine = 0;
    Matrix4x4 viewMatrix;
    int screenWidth = 1920;
    int screenHeight = 1080;
}

namespace Menu {
    std::vector<MenuItem> items = {
        {"ESP Enabled", &Config::ESPEnabled},
        {"Box ESP", &Config::BoxESP},
        {"Line ESP", &Config::LineESP},
        {"Health Bar", &Config::HealthBar},
        {"Name Tags", &Config::NameTags},
        {"Distance", &Config::Distance},
        {"Team Check", &Config::TeamCheck},
    };
    int selectedItem = 0;
}

// Global flag for running
bool g_Running = true;

// Main render loop
void RenderLoop() {
    while (g_Running) {
        // Handle menu input
        Menu::HandleInput();

        // Begin frame
        Renderer::BeginFrame();

        // Render ESP
        ESP::RenderAll();

        // Render menu
        Menu::Render();

        // End frame
        Renderer::EndFrame();

        // Small sleep to prevent 100% CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

// Message loop for the overlay window
void MessageLoop() {
    MSG msg;
    while (g_Running) {
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                g_Running = false;
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Show console for debugging
    AllocConsole();
    FILE* f;
    freopen_s(&f, "CONOUT$", "w", stdout);
    freopen_s(&f, "CONOUT$", "w", stderr);
    SetConsoleTitleA("icfox - Debug Console");

    std::cout << "=== icfox ESP ===" << std::endl;
    std::cout << "[*] Initializing..." << std::endl;

    // Initialize memory access to Roblox
    if (!Memory::Init()) {
        std::cerr << "[!] Failed to initialize memory access. Make sure Roblox is running." << std::endl;
        std::cout << "[*] Press any key to exit..." << std::endl;
        system("pause > nul");
        return 1;
    }

    // Initialize overlay window
    if (!Renderer::InitWindow(hInstance)) {
        std::cerr << "[!] Failed to create overlay window." << std::endl;
        Memory::Cleanup();
        std::cout << "[*] Press any key to exit..." << std::endl;
        system("pause > nul");
        return 1;
    }

    // Initialize DirectX 11
    if (!Renderer::InitD3D()) {
        std::cerr << "[!] Failed to initialize DirectX 11." << std::endl;
        Renderer::CleanupD3D();
        Memory::Cleanup();
        std::cout << "[*] Press any key to exit..." << std::endl;
        system("pause > nul");
        return 1;
    }

    std::cout << "[+] icfox ESP initialized successfully!" << std::endl;
    std::cout << "[*] Press INSERT to toggle menu" << std::endl;
    std::cout << "[*] Press ESC to exit" << std::endl;

    // Start render thread
    std::thread renderThread(RenderLoop);

    // Run message loop (main thread)
    MessageLoop();

    // Cleanup
    g_Running = false;
    if (renderThread.joinable()) {
        renderThread.join();
    }

    Renderer::CleanupD3D();
    Memory::Cleanup();

    if (f) fclose(f);
    FreeConsole();

    return 0;
}