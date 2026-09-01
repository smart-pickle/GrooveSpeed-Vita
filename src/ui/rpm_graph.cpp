#include "rpm_graph.hpp"
#include "../theme.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>

namespace GrooveUI {
    void renderRpmGraph(
        ImDrawList* drawList,
        ImVec2 position,
        ImVec2 size,
        const std::vector<float>& rpmHistory,
        float targetSpeed
    ) {
        if (!drawList || size.x <= 0 || size.y <= 0) return;

        ImVec2 maxPos(position.x + size.x, position.y + size.y);

        // 1. Dark Glassmorphic Backdrop
        drawList->AddRectFilled(position, maxPos, IM_COL32(11, 14, 19, 245), 8.0f);
        drawList->AddRect(position, maxPos, IM_COL32(40, 52, 68, 180), 8.0f, 0, 1.2f);

        // 2. Y-Axis Bounds (+/- 1.0 RPM for high resolution detail)
        float minY = targetSpeed - 1.0f;
        float maxY = targetSpeed + 1.0f;
        float rangeY = maxY - minY;

        // 3. Subtle Horizontal Reference Grid Lines
        for (float offset : {-0.75f, -0.5f, -0.25f, 0.25f, 0.5f, 0.75f}) {
            float yNorm = 1.0f - ((targetSpeed + offset - minY) / rangeY);
            float yPx = position.y + yNorm * size.y;
            drawList->AddLine(
                ImVec2(position.x + 4, yPx),
                ImVec2(maxPos.x - 4, yPx),
                IM_COL32(30, 38, 50, 140),
                1.0f
            );
        }

        // 4. Target Tolerance Ribbon (+/- 0.1% DIN Studio Tolerance Zone)
        float tolRpm = targetSpeed * 0.001f;
        float tolTopNorm = 1.0f - ((targetSpeed + tolRpm - minY) / rangeY);
        float tolBotNorm = 1.0f - ((targetSpeed - tolRpm - minY) / rangeY);
        float tolTopY = position.y + tolTopNorm * size.y;
        float tolBotY = position.y + tolBotNorm * size.y;

        drawList->AddRectFilled(
            ImVec2(position.x + 4, tolTopY),
            ImVec2(maxPos.x - 4, tolBotY),
            IM_COL32(0, 230, 118, 25)
        );

        // 5. Target Baseline (Gold Center Reference Line)
        float targetYNorm = 1.0f - ((targetSpeed - minY) / rangeY);
        float targetYPx = position.y + targetYNorm * size.y;
        drawList->AddLine(
            ImVec2(position.x + 4, targetYPx),
            ImVec2(maxPos.x - 4, targetYPx),
            IM_COL32(255, 215, 0, 160),
            1.2f
        );

        // Grid Labels
        char targetLabel[32];
        snprintf(targetLabel, sizeof(targetLabel), "TARGET %.2f", targetSpeed);
        drawList->AddText(ImVec2(position.x + 10, targetYPx - 14), IM_COL32(255, 215, 0, 160), targetLabel);

        if (rpmHistory.empty()) return;

        // 6. Waveform Calculations
        size_t n = rpmHistory.size();
        float stepX = (size.x - 16.0f) / static_cast<float>(std::max<size_t>(n - 1, 1));

        std::vector<ImVec2> points;
        points.reserve(n);

        for (size_t i = 0; i < n; ++i) {
            float val = std::clamp(rpmHistory[i], minY, maxY);
            float normY = 1.0f - ((val - minY) / rangeY);
            float px = position.x + 8.0f + static_cast<float>(i) * stepX;
            float py = position.y + normY * size.y;
            points.push_back(ImVec2(px, py));
        }

        // 7. Shaded Translucent Area Fill Beneath Curve
        float bottomY = position.y + size.y - 4.0f;
        for (size_t i = 0; i < points.size() - 1; ++i) {
            ImVec2 p1 = points[i];
            ImVec2 p2 = points[i + 1];
            ImVec2 b1(p1.x, bottomY);
            ImVec2 b2(p2.x, bottomY);

            drawList->AddQuadFilled(p1, p2, b2, b1, IM_COL32(0, 229, 255, 18));
        }

        // 8. Waveform Line (Glow Halo + Sharp Core)
        for (size_t i = 0; i < points.size() - 1; ++i) {
            // Glow layer
            drawList->AddLine(points[i], points[i + 1], IM_COL32(0, 229, 255, 70), 4.5f);
            // Sharp core
            drawList->AddLine(points[i], points[i + 1], IM_COL32(0, 229, 255, 255), 2.0f);
        }

        // 9. Leading-Edge Beacon (Latest Sample Ping Dot)
        if (!points.empty()) {
            ImVec2 lastPt = points.back();
            // Outer glow ring
            drawList->AddCircle(lastPt, 6.0f, IM_COL32(0, 229, 255, 120), 16, 1.5f);
            // Inner solid dot
            drawList->AddCircleFilled(lastPt, 3.5f, IM_COL32(255, 255, 255, 255), 16);
            drawList->AddCircleFilled(lastPt, 2.0f, IM_COL32(0, 229, 255, 255), 16);
        }
    }
}
