#ifndef THEME_HPP
#define THEME_HPP

#include "imgui.h"

namespace GrooveTheme {
    // Custom Hi-Fi Dark Theme for ImGui
    void applyTheme();
    
    // Compile-time constant color definitions (Zero runtime .init_array static constructors)
    constexpr ImVec4 AccentTeal (0.149f, 0.776f, 0.855f, 1.00f); // #26C6DA Primary Cyan
    constexpr ImVec4 AccentGreen(0.000f, 0.902f, 0.463f, 1.00f); // #00E676 Emerald Green
    constexpr ImVec4 AccentGold (1.000f, 0.792f, 0.157f, 1.00f); // #FFCA28 Secondary Gold
    constexpr ImVec4 AccentRed  (0.957f, 0.263f, 0.212f, 1.00f); // #F44336 Error Red
    constexpr ImVec4 CardBg     (0.082f, 0.102f, 0.129f, 0.95f); // #151A21 Surface Card
    constexpr ImVec4 CardElevated(0.118f, 0.149f, 0.188f, 0.95f); // #1E2630 Hero Card
    constexpr ImVec4 BorderSubtle(0.180f, 0.235f, 0.290f, 0.60f); // #2E3C4A Border
    constexpr ImVec4 BorderBright(0.149f, 0.776f, 0.855f, 0.40f); // Subtle Cyan Glow
    constexpr ImVec4 TextBright (0.925f, 0.937f, 0.945f, 1.00f); // #ECEFF1 OnBackground
    constexpr ImVec4 TextMuted  (0.600f, 0.670f, 0.720f, 1.00f); // #99AB8
    constexpr ImVec4 TextDim    (0.400f, 0.470f, 0.520f, 1.00f); // #667885

    // UI Helper: Render a styled segmented pill button
    bool renderPillButton(const char* label, bool isSelected, ImVec2 size);

    // UI Helper: Render a telemetry meter bar
    void renderLevelMeter(const char* label, float value, float maxValue, const char* unitStr, ImVec4 color, float width);
}

#endif // THEME_HPP
