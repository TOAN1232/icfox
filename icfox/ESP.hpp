#pragma once
#include "Entities.hpp"
#include "Renderer.hpp"
#include "Config.hpp"
#include "Math.hpp"
#include <string>
#include <sstream>
#include <iomanip>

namespace ESP {
    void RenderPlayer(const PlayerEntity& player, const PlayerEntity& local) {
        if (!Config::ESPEnabled) return;
        if (!player.isAlive) return;
        if (player.rootPart == 0 || player.head == 0) return;

        // World to screen for head and root
        Vector2 headScreen, rootScreen;
        if (!Math::WorldToScreen(player.headPosition, headScreen, Entities::viewMatrix,
            Entities::screenWidth, Entities::screenHeight))
            return;
        if (!Math::WorldToScreen(player.rootPosition, rootScreen, Entities::viewMatrix,
            Entities::screenWidth, Entities::screenHeight))
            return;

        // Determine color based on team
        uint32_t espColor;
        if (Config::TeamCheck && player.team == local.team) {
            espColor = Config::TeamColor;
        } else {
            espColor = Config::EnemyColor;
        }

        // Calculate box dimensions
        float boxHeight = abs(headScreen.y - rootScreen.y);
        float boxWidth = boxHeight * 0.5f;
        if (boxWidth < 10.0f) boxWidth = 10.0f;

        float boxX = rootScreen.x - boxWidth / 2.0f;
        float boxY = headScreen.y;

        // === Box ESP ===
        if (Config::BoxESP) {
            Renderer::DrawRect(boxX, boxY, boxWidth, boxHeight, espColor);
        }

        // === Line ESP (Snap Line) ===
        if (Config::LineESP) {
            float screenBottom = (float)Entities::screenHeight;
            Renderer::DrawLine(rootScreen.x, rootScreen.y, rootScreen.x, screenBottom, espColor);
        }

        // === Health Bar ===
        if (Config::HealthBar && player.maxHealth > 0) {
            float healthPercent = player.health / player.maxHealth;
            if (healthPercent < 0.0f) healthPercent = 0.0f;
            if (healthPercent > 1.0f) healthPercent = 1.0f;

            // Health bar on the left side of the box
            float barWidth = 4.0f;
            float barX = boxX - barWidth - 2.0f;
            float barY = boxY;
            float barHeight = boxHeight;

            // Background (black)
            Renderer::DrawFilledRect(barX, barY, barWidth, barHeight, COLOR_A(0, 0, 0, 200));

            // Health fill
            float fillHeight = barHeight * healthPercent;
            float fillY = barY + (barHeight - fillHeight);

            uint32_t healthColor;
            if (healthPercent > 0.5f) {
                healthColor = Config::HealthHigh;
            } else if (healthPercent > 0.25f) {
                healthColor = Config::HealthMid;
            } else {
                healthColor = Config::HealthLow;
            }

            Renderer::DrawFilledRect(barX + 1.0f, fillY, barWidth - 2.0f, fillHeight - 1.0f, healthColor);
        }

        // === Name Tags ===
        if (Config::NameTags) {
            std::string displayName = player.displayName.empty() ? player.name : player.displayName;
            Renderer::DrawString(displayName, rootScreen.x, boxY - 14.0f, Config::TextColor, true, false);
        }

        // === Distance ===
        if (Config::Distance) {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(0) << player.distance << " studs";
            Renderer::DrawString(ss.str(), rootScreen.x, rootScreen.y + 2.0f, Config::TextColor, true, true);
        }
    }

    void RenderAll() {
        // Update entities each frame
        Entities::UpdateEntities();

        const auto& localPlayer = Entities::localPlayer;
        const auto& players = Entities::players;

        for (const auto& player : players) {
            RenderPlayer(player, localPlayer);
        }
    }
}