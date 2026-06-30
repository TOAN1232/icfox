#pragma once
#include "Renderer.hpp"
#include "Config.hpp"
#include <string>
#include <vector>

namespace Menu {
    struct MenuItem {
        std::string label;
        bool* value;
    };

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
    const int menuX = 20;
    const int menuY = 20;
    const int itemHeight = 20;
    const int menuWidth = 200;

    void Render() {
        if (!Config::MenuOpen) return;

        // Menu background
        int totalHeight = (int)items.size() * itemHeight + 40;
        Renderer::DrawFilledRect((float)menuX, (float)menuY, (float)menuWidth, (float)totalHeight,
            COLOR_A(30, 30, 30, 200));
        Renderer::DrawRect((float)menuX, (float)menuY, (float)menuWidth, (float)totalHeight,
            COLOR_A(100, 100, 100, 255));

        // Title
        Renderer::DrawString("icfox ESP", (float)(menuX + menuWidth / 2), (float)(menuY + 5),
            COLOR_A(0, 200, 255, 255), true, false);

        // Items
        for (size_t i = 0; i < items.size(); i++) {
            int yPos = menuY + 30 + (int)i * itemHeight;

            // Highlight selected item
            if ((int)i == selectedItem) {
                Renderer::DrawFilledRect((float)(menuX + 2), (float)yPos, (float)(menuWidth - 4), (float)itemHeight,
                    COLOR_A(50, 50, 50, 100));
            }

            // Draw toggle indicator
            std::string status = *items[i].value ? "[ON] " : "[OFF] ";
            uint32_t statusColor = *items[i].value ?
                COLOR_A(0, 255, 0, 255) :
                COLOR_A(255, 0, 0, 255);

            Renderer::DrawString(status + items[i].label, (float)(menuX + 10), (float)(yPos + 2),
                statusColor, false, false);
        }

        // Footer hint
        Renderer::DrawString("Arrow keys: Navigate  |  Enter: Toggle  |  Insert: Close",
            (float)(menuX + menuWidth / 2), (float)(menuY + totalHeight - 15),
            COLOR_A(200, 200, 200, 200), true, true);
    }

    void HandleInput() {
        if (!Config::MenuOpen) return;

        if (GetAsyncKeyState(VK_UP) & 1) {
            selectedItem--;
            if (selectedItem < 0) selectedItem = (int)items.size() - 1;
        }
        if (GetAsyncKeyState(VK_DOWN) & 1) {
            selectedItem++;
            if (selectedItem >= (int)items.size()) selectedItem = 0;
        }
        if (GetAsyncKeyState(VK_RETURN) & 1) {
            if (selectedItem >= 0 && selectedItem < (int)items.size()) {
                *items[selectedItem].value = !*items[selectedItem].value;
            }
        }
    }
}