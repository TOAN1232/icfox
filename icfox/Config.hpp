#pragma once
#include <cstdint>
#include <windows.h>

// Color macro (ARGB format)
#define COLOR_A(r, g, b, a) (((a) & 0xFF) << 24) | (((r) & 0xFF) << 16) | (((g) & 0xFF) << 8) | ((b) & 0xFF)
#define COLOR_RGB(r, g, b) COLOR_A(r, g, b, 255)

namespace Config {
    // Window
    inline constexpr const wchar_t* WindowClassName = L"icfox_overlay";
    inline constexpr const wchar_t* WindowTitle = L"icfox";
    inline int WindowWidth = 1920;
    inline int WindowHeight = 1080;

    // Colors (ARGB format via uint32_t)
    inline constexpr uint32_t EnemyColor = COLOR_RGB(255, 50, 50);
    inline constexpr uint32_t TeamColor = COLOR_RGB(50, 255, 50);
    inline constexpr uint32_t BoxColor = COLOR_RGB(255, 255, 255);
    inline constexpr uint32_t LineColor = COLOR_RGB(255, 255, 255);
    inline constexpr uint32_t TextColor = COLOR_RGB(255, 255, 255);
    inline constexpr uint32_t HealthLow = COLOR_RGB(255, 0, 0);
    inline constexpr uint32_t HealthMid = COLOR_RGB(255, 255, 0);
    inline constexpr uint32_t HealthHigh = COLOR_RGB(0, 255, 0);

    // Default toggle states
    inline bool ESPEnabled = true;
    inline bool BoxESP = true;
    inline bool LineESP = true;
    inline bool HealthBar = true;
    inline bool NameTags = true;
    inline bool Distance = true;
    inline bool TeamCheck = true;
    inline bool MenuOpen = true;

    // Keybinds
    inline constexpr int MenuToggleKey = VK_INSERT;
}