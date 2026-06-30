#pragma once
#include <windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <dwmapi.h>
#include <string>
#include "Config.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwmapi.lib")

namespace Renderer {
    // DX11 objects
    IDXGISwapChain* swapChain = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    ID3D11RenderTargetView* renderTargetView = nullptr;
    ID3D11Texture2D* backBuffer = nullptr;

    // Window
    HWND overlayWindow = nullptr;
    HWND targetWindow = nullptr;
    WNDCLASSEX wc = {};

    struct Vertex {
        float x, y, z;
        uint32_t color;
    };

    ID3D11Buffer* lineBuffer = nullptr;
    ID3D11Buffer* rectBuffer = nullptr;

    bool InitWindow(HINSTANCE instance);
    bool InitD3D();
    void CleanupD3D();
    void BeginFrame();
    void EndFrame();
    void DrawLine(float x1, float y1, float x2, float y2, uint32_t color, float thickness = 1.0f);
    void DrawRect(float x, float y, float w, float h, uint32_t color, float thickness = 1.0f);
    void DrawFilledRect(float x, float y, float w, float h, uint32_t color);
    void DrawString(const std::string& text, float x, float y, uint32_t color, bool centered = false, bool small = false);
    void DrawCircle(float cx, float cy, float radius, uint32_t color, int segments = 32);
    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
}