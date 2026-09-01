#include "polar_plot.hpp"
#include "../theme.hpp"
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

namespace GrooveUI {
    void renderPolarPlot(
        ImDrawList* drawList,
        ImVec2 center,
        float radius,
        const std::vector<MeasurementPoint>& samples,
        float targetSpeed
    ) {
        if (!drawList) return;

        // 1. Dark Base Platter Background
        drawList->AddCircleFilled(center, radius, IM_COL32(11, 14, 19, 245), 64);
        drawList->AddCircle(center, radius, IM_COL32(40, 52, 68, 200), 64, 1.5f);

        // 2. Tolerance Rings (±1.0%, ±0.5%, 0% Target)
        float rTarget = radius * 0.66f;
        float rPlusHalf = radius * 0.83f;
        float rMinusHalf = radius * 0.49f;

        // Soft Green In-Spec Safe Zone Fill (Between -0.5% and +0.5%)
        drawList->AddCircleFilled(center, rPlusHalf, IM_COL32(0, 230, 118, 12), 48);
        drawList->AddCircleFilled(center, rMinusHalf, IM_COL32(11, 14, 19, 245), 48);

        // Concentric Guides
        drawList->AddCircle(center, rPlusHalf, IM_COL32(0, 230, 118, 45), 48, 1.0f);
        drawList->AddCircle(center, rMinusHalf, IM_COL32(0, 230, 118, 45), 48, 1.0f);
        drawList->AddCircle(center, rTarget, IM_COL32(255, 215, 0, 180), 48, 1.2f); // 0% Gold Target Line

        // 3. Radial Crosshairs (0°, 90°, 180°, 270°)
        drawList->AddLine(ImVec2(center.x - radius, center.y), ImVec2(center.x + radius, center.y), IM_COL32(45, 58, 75, 140), 1.0f);
        drawList->AddLine(ImVec2(center.x, center.y - radius), ImVec2(center.x, center.y + radius), IM_COL32(45, 58, 75, 140), 1.0f);

        // Cardinal Degree Labels
        drawList->AddText(ImVec2(center.x - 8, center.y - radius + 4), IM_COL32(140, 160, 185, 180), "0°");
        drawList->AddText(ImVec2(center.x + radius - 24, center.y - 8), IM_COL32(140, 160, 185, 180), "90°");
        drawList->AddText(ImVec2(center.x - 12, center.y + radius - 18), IM_COL32(140, 160, 185, 180), "180°");
        drawList->AddText(ImVec2(center.x - radius + 4, center.y - 8), IM_COL32(140, 160, 185, 180), "270°");

        // Center Spindle
        drawList->AddCircleFilled(center, radius * 0.12f, IM_COL32(35, 42, 55, 255), 24);
        drawList->AddCircle(center, radius * 0.12f, IM_COL32(80, 95, 120, 200), 24, 1.0f);

        if (samples.empty()) return;

        // 4. Plot Deviation Points & Rotational Waveform
        std::vector<ImVec2> polarPoints;
        polarPoints.reserve(samples.size());

        for (const auto& pt : samples) {
            float rad = (pt.angle - 90.0f) * (M_PI / 180.0f); // 0 deg at top
            float devPercent = ((pt.rpm - targetSpeed) / targetSpeed) * 100.0f; // e.g. +0.15%

            // Map deviation to radial distance: 0% dev = 0.66 * radius, +/-1.5% max
            float rNorm = 0.66f + (devPercent / 3.0f) * 0.34f;
            float ptRadius = radius * std::clamp(rNorm, 0.15f, 0.98f);

            ImVec2 ptPos(center.x + std::cos(rad) * ptRadius, center.y + std::sin(rad) * ptRadius);
            polarPoints.push_back(ptPos);

            ImU32 dotColor;
            if (std::abs(devPercent) < 0.10f) {
                dotColor = IM_COL32(0, 229, 255, 200); // Neon Cyan
            } else if (std::abs(devPercent) < 0.25f) {
                dotColor = IM_COL32(255, 215, 0, 220); // Amber Gold
            } else {
                dotColor = IM_COL32(255, 82, 82, 230);  // Coral Red
            }

            drawList->AddCircleFilled(ptPos, 2.5f, dotColor, 8);
        }

        // Trace line connecting rotational samples
        if (polarPoints.size() > 1) {
            for (size_t i = 0; i < polarPoints.size() - 1; ++i) {
                drawList->AddLine(polarPoints[i], polarPoints[i + 1], IM_COL32(0, 229, 255, 60), 1.0f);
            }
        }
    }
}
