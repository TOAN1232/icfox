#include "Renderer.hpp"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <string>
#include <vector>
#include <cstdint>
#include <cmath>
#include <cstring>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwmapi.lib")

namespace Renderer {
    // Global variable definitions
    IDXGISwapChain* swapChain = nullptr;
    ID3D11Device* device = nullptr;
    ID3D11DeviceContext* context = nullptr;
    ID3D11RenderTargetView* renderTargetView = nullptr;
    ID3D11Texture2D* backBuffer = nullptr;
    HWND overlayWindow = nullptr;
    HWND targetWindow = nullptr;
    WNDCLASSEX wc = {};
    ID3D11Buffer* lineBuffer = nullptr;
    ID3D11Buffer* rectBuffer = nullptr;

    ID3D11VertexShader* g_vertexShader = nullptr;
    ID3D11PixelShader* g_pixelShader = nullptr;
    ID3D11InputLayout* g_inputLayout = nullptr;

    // Minimal HLSL vertex shader
    const char* g_vsHLSL = R"(
        struct VS_IN {
            float3 pos : POSITION;
            float4 color : COLOR;
        };
        struct PS_IN {
            float4 pos : SV_Position;
            float4 color : COLOR;
        };
        PS_IN main(VS_IN input) {
            PS_IN output;
            output.pos = float4(input.pos.x, input.pos.y, 0.0f, 1.0f);
            output.color = input.color;
            return output;
        }
    )";

    // Minimal HLSL pixel shader
    const char* g_psHLSL = R"(
        struct PS_IN {
            float4 pos : SV_Position;
            float4 color : COLOR;
        };
        float4 main(PS_IN input) : SV_Target {
            return input.color;
        }
    )";

    bool CreateShaders() {
        ID3DBlob* vsBlob = nullptr;
        ID3DBlob* psBlob = nullptr;
        ID3DBlob* errorBlob = nullptr;

        HRESULT hr = D3DCompile(g_vsHLSL, strlen(g_vsHLSL), "vs", nullptr, nullptr,
            "main", "vs_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &vsBlob, &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob) {
                OutputDebugStringA((const char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }
            return false;
        }

        hr = D3DCompile(g_psHLSL, strlen(g_psHLSL), "ps", nullptr, nullptr,
            "main", "ps_4_0", D3DCOMPILE_OPTIMIZATION_LEVEL3, 0, &psBlob, &errorBlob);
        if (FAILED(hr)) {
            if (errorBlob) {
                OutputDebugStringA((const char*)errorBlob->GetBufferPointer());
                errorBlob->Release();
            }
            vsBlob->Release();
            return false;
        }

        hr = device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(),
            nullptr, &g_vertexShader);
        if (FAILED(hr)) {
            vsBlob->Release();
            psBlob->Release();
            return false;
        }

        hr = device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(),
            nullptr, &g_pixelShader);
        if (FAILED(hr)) {
            vsBlob->Release();
            psBlob->Release();
            return false;
        }

        // Create input layout
        D3D11_INPUT_ELEMENT_DESC layout[] = {
            { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        };

        hr = device->CreateInputLayout(layout, 2, vsBlob->GetBufferPointer(),
            vsBlob->GetBufferSize(), &g_inputLayout);
        if (FAILED(hr)) {
            vsBlob->Release();
            psBlob->Release();
            return false;
        }

        vsBlob->Release();
        psBlob->Release();
        return true;
    }

    bool InitWindow(HINSTANCE instance) {
        targetWindow = Memory::hwnd;
        if (!targetWindow) return false;

        wc = {};
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WndProc;
        wc.cbClsExtra = 0;
        wc.cbWndExtra = 0;
        wc.hInstance = instance;
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)CreateSolidBrush(RGB(0, 0, 0));
        wc.lpszClassName = Config::WindowClassName;

        if (!RegisterClassEx(&wc)) {
            return false;
        }

        RECT rect;
        GetWindowRect(targetWindow, &rect);

        overlayWindow = CreateWindowEx(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW,
            Config::WindowClassName,
            Config::WindowTitle,
            WS_POPUP,
            rect.left, rect.top,
            rect.right - rect.left, rect.bottom - rect.top,
            nullptr, nullptr,
            instance,
            nullptr
        );

        if (!overlayWindow) return false;

        SetLayeredWindowAttributes(overlayWindow, RGB(0, 0, 0), 0, LWA_COLORKEY);
        SetWindowLong(overlayWindow, GWL_EXSTYLE, GetWindowLong(overlayWindow, GWL_EXSTYLE) | WS_EX_LAYERED);
        SetLayeredWindowAttributes(overlayWindow, 0, 255, LWA_ALPHA);
        SetWindowLong(overlayWindow, GWL_EXSTYLE, GetWindowLong(overlayWindow, GWL_EXSTYLE) | WS_EX_TRANSPARENT);

        ShowWindow(overlayWindow, SW_SHOW);
        SetForegroundWindow(overlayWindow);

        DWM_BLURBEHIND bb = { sizeof(bb) };
        bb.dwFlags = DWM_BB_ENABLE | DWM_BB_BLURREGION;
        bb.hRgnBlur = CreateRectRgn(0, 0, -1, -1);
        bb.fEnable = TRUE;
        DwmEnableBlurBehindWindow(overlayWindow, &bb);
        DeleteObject(bb.hRgnBlur);

        return true;
    }

    bool InitD3D() {
        DXGI_SWAP_CHAIN_DESC scd = {};
        scd.BufferCount = 1;
        scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        scd.BufferDesc.Width = Config::WindowWidth;
        scd.BufferDesc.Height = Config::WindowHeight;
        scd.BufferDesc.RefreshRate.Numerator = 60;
        scd.BufferDesc.RefreshRate.Denominator = 1;
        scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
        scd.OutputWindow = overlayWindow;
        scd.SampleDesc.Count = 1;
        scd.SampleDesc.Quality = 0;
        scd.Windowed = TRUE;
        scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

        D3D_FEATURE_LEVEL featureLevel;
        HRESULT hr = D3D11CreateDeviceAndSwapChain(
            nullptr,
            D3D_DRIVER_TYPE_HARDWARE,
            nullptr,
            D3D11_CREATE_DEVICE_BGRA_SUPPORT,
            nullptr,
            0,
            D3D11_SDK_VERSION,
            &scd,
            &swapChain,
            &device,
            &featureLevel,
            &context
        );

        if (FAILED(hr)) return false;

        hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
        if (FAILED(hr)) return false;

        hr = device->CreateRenderTargetView(backBuffer, nullptr, &renderTargetView);
        if (FAILED(hr)) return false;

        D3D11_VIEWPORT viewport = {};
        viewport.Width = (FLOAT)Config::WindowWidth;
        viewport.Height = (FLOAT)Config::WindowHeight;
        viewport.MinDepth = 0.0f;
        viewport.MaxDepth = 1.0f;
        viewport.TopLeftX = 0;
        viewport.TopLeftY = 0;
        context->RSSetViewports(1, &viewport);

        // Create vertex buffer for lines (dynamic, 2 vertices)
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = sizeof(Vertex) * 2;
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        device->CreateBuffer(&bd, nullptr, &lineBuffer);

        // Create vertex buffer for rect (5 vertices for a closed loop)
        bd.ByteWidth = sizeof(Vertex) * 5;
        device->CreateBuffer(&bd, nullptr, &rectBuffer);

        // Create shaders
        if (!CreateShaders()) {
            return false;
        }

        return true;
    }

    void CleanupD3D() {
        if (g_inputLayout) { g_inputLayout->Release(); g_inputLayout = nullptr; }
        if (g_vertexShader) { g_vertexShader->Release(); g_vertexShader = nullptr; }
        if (g_pixelShader) { g_pixelShader->Release(); g_pixelShader = nullptr; }
        if (lineBuffer) { lineBuffer->Release(); lineBuffer = nullptr; }
        if (rectBuffer) { rectBuffer->Release(); rectBuffer = nullptr; }
        if (renderTargetView) { renderTargetView->Release(); renderTargetView = nullptr; }
        if (backBuffer) { backBuffer->Release(); backBuffer = nullptr; }
        if (context) { context->Release(); context = nullptr; }
        if (swapChain) { swapChain->Release(); swapChain = nullptr; }
        if (device) { device->Release(); device = nullptr; }
        if (overlayWindow) {
            DestroyWindow(overlayWindow);
            overlayWindow = nullptr;
        }
        UnregisterClass(Config::WindowClassName, GetModuleHandle(nullptr));
    }

    void BeginFrame() {
        if (targetWindow && overlayWindow) {
            RECT rect;
            GetWindowRect(targetWindow, &rect);
            SetWindowPos(overlayWindow, HWND_TOPMOST, rect.left, rect.top,
                rect.right - rect.left, rect.bottom - rect.top, SWP_SHOWWINDOW);

            Config::WindowWidth = rect.right - rect.left;
            Config::WindowHeight = rect.bottom - rect.top;
        }

        float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        context->OMSetRenderTargets(1, &renderTargetView, nullptr);
        context->ClearRenderTargetView(renderTargetView, clearColor);

        // Bind shaders and input layout
        context->VSSetShader(g_vertexShader, nullptr, 0);
        context->PSSetShader(g_pixelShader, nullptr, 0);
        context->IASetInputLayout(g_inputLayout);
    }

    void EndFrame() {
        swapChain->Present(1, 0);
    }

    void DrawLine(float x1, float y1, float x2, float y2, uint32_t color, float thickness) {
        if (!lineBuffer || !context) return;

        D3D11_MAPPED_SUBRESOURCE mapped;
        if (FAILED(context->Map(lineBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            return;

        Vertex* verts = (Vertex*)mapped.pData;
        verts[0] = { x1, y1, 0.0f, color };
        verts[1] = { x2, y2, 0.0f, color };

        context->Unmap(lineBuffer, 0);

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &lineBuffer, &stride, &offset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        context->Draw(2, 0);
    }

    void DrawRect(float x, float y, float w, float h, uint32_t color, float thickness) {
        DrawLine(x, y, x + w, y, color, thickness);
        DrawLine(x + w, y, x + w, y + h, color, thickness);
        DrawLine(x + w, y + h, x, y + h, color, thickness);
        DrawLine(x, y + h, x, y, color, thickness);
    }

    void DrawFilledRect(float x, float y, float w, float h, uint32_t color) {
        if (!rectBuffer || !context) return;

        D3D11_MAPPED_SUBRESOURCE mapped;
        if (FAILED(context->Map(rectBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped)))
            return;

        Vertex* verts = (Vertex*)mapped.pData;
        verts[0] = { x, y, 0.0f, color };
        verts[1] = { x + w, y, 0.0f, color };
        verts[2] = { x + w, y + h, 0.0f, color };
        verts[3] = { x, y + h, 0.0f, color };
        verts[4] = { x, y, 0.0f, color };

        context->Unmap(rectBuffer, 0);

        UINT stride = sizeof(Vertex);
        UINT offset = 0;
        context->IASetVertexBuffers(0, 1, &rectBuffer, &stride, &offset);
        context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
        context->Draw(5, 0);
    }

    // Helper: extract ARGB components
    inline uint8_t GetA(uint32_t color) { return (uint8_t)((color >> 24) & 0xFF); }
    inline uint8_t GetR(uint32_t color) { return (uint8_t)((color >> 16) & 0xFF); }
    inline uint8_t GetG(uint32_t color) { return (uint8_t)((color >> 8) & 0xFF); }
    inline uint8_t GetB(uint32_t color) { return (uint8_t)(color & 0xFF); }

    void DrawString(const std::string& text, float x, float y, uint32_t color, bool centered, bool small) {
        if (!device || !context || !backBuffer) return;

        // Use IDXGISurface1::GetDC to draw text directly onto the backbuffer
        IDXGISurface1* dxgiSurface = nullptr;
        if (FAILED(backBuffer->QueryInterface(__uuidof(IDXGISurface1), (void**)&dxgiSurface)))
            return;

        HDC surfaceDC = nullptr;
        if (FAILED(dxgiSurface->GetDC(FALSE, &surfaceDC))) {
            dxgiSurface->Release();
            return;
        }

        // Convert to wide string
        int wideLen = MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, nullptr, 0);
        if (wideLen > 0) {
            std::wstring wideText(wideLen, L'\0');
            MultiByteToWideChar(CP_UTF8, 0, text.c_str(), -1, &wideText[0], wideLen);

            SetBkMode(surfaceDC, TRANSPARENT);

            HFONT hFont = CreateFontA(
                small ? -11 : -14, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Verdana");
            HFONT oldFont = (HFONT)SelectObject(surfaceDC, hFont);

            // Calculate text size for centering
            SIZE textSize = {};
            if (centered) {
                GetTextExtentPoint32W(surfaceDC, wideText.c_str(), (int)wideText.length() - 1, &textSize);
                x -= (float)textSize.cx / 2.0f;
                y -= (float)textSize.cy / 2.0f;
            }

            // Draw shadow
            SetTextColor(surfaceDC, RGB(0, 0, 0));
            TextOutW(surfaceDC, (int)x + 1, (int)y + 1, wideText.c_str(), (int)wideText.length() - 1);

            // Draw text
            SetTextColor(surfaceDC, RGB(GetR(color), GetG(color), GetB(color)));
            TextOutW(surfaceDC, (int)x, (int)y, wideText.c_str(), (int)wideText.length() - 1);

            SelectObject(surfaceDC, oldFont);
            DeleteObject(hFont);
        }

        dxgiSurface->ReleaseDC(nullptr);
        dxgiSurface->Release();
    }

    void DrawCircle(float cx, float cy, float radius, uint32_t color, int segments) {
        if (!lineBuffer || !context) return;

        std::vector<Vertex> verts;
        verts.resize(segments + 1);

        float step = 2.0f * 3.14159265f / segments;
        for (int i = 0; i <= segments; i++) {
            float angle = i * step;
            verts[i] = {
                cx + radius * cosf(angle),
                cy + radius * sinf(angle),
                0.0f,
                color
            };
        }

        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = (UINT)(sizeof(Vertex) * verts.size());
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA sd = { verts.data(), 0, 0 };
        ID3D11Buffer* circleBuffer = nullptr;
        device->CreateBuffer(&bd, &sd, &circleBuffer);

        if (circleBuffer) {
            UINT stride = sizeof(Vertex);
            UINT offset = 0;
            context->IASetVertexBuffers(0, 1, &circleBuffer, &stride, &offset);
            context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINESTRIP);
            context->Draw((UINT)verts.size(), 0);
            circleBuffer->Release();
        }
    }

    LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
        switch (msg) {
            case WM_KEYDOWN:
                if (wParam == Config::MenuToggleKey) {
                    Config::MenuOpen = !Config::MenuOpen;
                }
                break;
            case WM_DESTROY:
                PostQuitMessage(0);
                break;
            default:
                return DefWindowProc(hwnd, msg, wParam, lParam);
        }
        return 0;
    }
}