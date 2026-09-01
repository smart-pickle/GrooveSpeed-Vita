#include "strobe_ring.hpp"
#include "../theme.hpp"
#include <cmath>
#include <cstdio>
#include <initializer_list>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

namespace GrooveUI {
    void renderStrobeRing(
        ImDrawList* drawList,
        ImVec2 center,
        float radius,
        float targetSpeed,
        float currentRpm,
        float rotationAngleDeg
    ) {
        if (!drawList) return;

        // 1. Soft Ambient Outer Halo
        drawList->AddCircleFilled(center, radius + 8.0f, IM_COL32(0, 229, 255, 12), 64);
        drawList->AddCircleFilled(center, radius + 4.0f, IM_COL32(0, 229, 255, 20), 64);

        // 2. Outer Machined Aluminum Platter Rim
        drawList->AddCircleFilled(center, radius, IM_COL32(14, 17, 23, 255), 64);
        drawList->AddCircle(center, radius, IM_COL32(55, 68, 86, 255), 64, 3.5f);
        drawList->AddCircle(center, radius - 2.0f, IM_COL32(25, 32, 42, 255), 64, 1.5f);

        // 3. Realistic Concentric Vinyl Grooves (Micro-sheen rings)
        for (float rFactor : {0.94f, 0.88f, 0.82f, 0.76f, 0.70f, 0.64f, 0.58f, 0.52f, 0.46f, 0.40f}) {
            drawList->AddCircle(center, radius * rFactor, IM_COL32(38, 48, 62, 60), 64, 1.0f);
        }

        // 4. Strobe Track Ticks
        int numTicks = 72;
        if (std::abs(targetSpeed - 45.0f) < 1.0f) numTicks = 54;
        else if (std::abs(targetSpeed - 78.0f) < 1.0f) numTicks = 36;

        float angleOffsetRad = rotationAngleDeg * (M_PI / 180.0f);
        float innerR = radius * 0.74f;
        float outerR = radius * 0.93f;

        // Dynamic Speed Lock Color with Smooth Heatmap Transitions
        float speedRatio = (currentRpm > 0.1f) ? (currentRpm / targetSpeed) : 1.0f;
        float dev = std::abs(speedRatio - 1.0f);

        ImU32 tickGlowColor;
        ImU32 tickCoreColor;

        if (dev < 0.0015f) {
            // Perfect Lock: Emerald Green
            tickGlowColor = IM_COL32(0, 230, 118, 90);
            tickCoreColor = IM_COL32(0, 230, 118, 255);
        } else if (dev < 0.0050f) {
            // In Spec (<0.5%): Neon Cyan
            tickGlowColor = IM_COL32(0, 229, 255, 90);
            tickCoreColor = IM_COL32(0, 229, 255, 255);
        } else if (dev < 0.0150f) {
            // Minor Drift: Warm Gold
            tickGlowColor = IM_COL32(255, 215, 0, 90);
            tickCoreColor = IM_COL32(255, 215, 0, 255);
        } else {
            // Significant Drift: Coral Red
            tickGlowColor = IM_COL32(255, 82, 82, 90);
            tickCoreColor = IM_COL32(255, 82, 82, 255);
        }

        // Render Strobe Ticks (Layer 1: Glow Halo, Layer 2: Sharp Core)
        for (int i = 0; i < numTicks; ++i) {
            float baseAngle = (2.0f * M_PI * i) / static_cast<float>(numTicks);
            float a = baseAngle + angleOffsetRad;

            ImVec2 p1(center.x + std::cos(a) * innerR, center.y + std::sin(a) * innerR);
            ImVec2 p2(center.x + std::cos(a) * outerR, center.y + std::sin(a) * outerR);

            // Glow layer
            drawList->AddLine(p1, p2, tickGlowColor, 4.0f);
            // Core sharp tick
            drawList->AddLine(p1, p2, tickCoreColor, 2.0f);
        }

        // 5. Center Vinyl Label Disc (Rich midnight blue with gold rim)
        float labelRadius = radius * 0.35f;
        drawList->AddCircleFilled(center, labelRadius, IM_COL32(10, 16, 26, 255), 48);
        drawList->AddCircle(center, labelRadius, IM_COL32(255, 215, 0, 180), 48, 1.5f);
        drawList->AddCircle(center, labelRadius - 4.0f, IM_COL32(0, 229, 255, 80), 48, 1.0f);

        // Speed Label Text on Disc
        char speedLabel[16];
        if (std::abs(targetSpeed - 33.33f) < 0.1f) snprintf(speedLabel, sizeof(speedLabel), "33 1/3");
        else if (std::abs(targetSpeed - 45.0f) < 0.1f) snprintf(speedLabel, sizeof(speedLabel), "45 RPM");
        else snprintf(speedLabel, sizeof(speedLabel), "78 RPM");

        ImVec2 textSize = ImGui::CalcTextSize(speedLabel);
        drawList->AddText(
            ImVec2(center.x - textSize.x * 0.5f, center.y - labelRadius * 0.55f - textSize.y * 0.5f),
            IM_COL32(200, 220, 240, 220),
            speedLabel
        );

        // 6. Brushed Metallic Spindle & 45-Adapter Cap
        float spindleRadius = radius * 0.14f;
        drawList->AddCircleFilled(center, spindleRadius, IM_COL32(40, 48, 60, 255), 32);
        drawList->AddCircle(center, spindleRadius, IM_COL32(120, 140, 165, 240), 32, 2.0f);
        drawList->AddCircle(center, spindleRadius * 0.6f, IM_COL32(70, 85, 105, 200), 24, 1.2f);

        // Center Spindle Pin (Polished Chrome)
        drawList->AddCircleFilled(center, spindleRadius * 0.28f, IM_COL32(240, 245, 255, 255), 16);
        drawList->AddCircle(center, spindleRadius * 0.28f, IM_COL32(100, 120, 140, 255), 16, 1.0f);
    }
}
