#include "theme.hpp"
#include <cstdio>
#include <algorithm>

namespace GrooveTheme {
    void applyTheme() {
        ImGuiStyle& style = ImGui::GetStyle();

        // Refined Rounding & Padding for PS Vita 960x544 screen
        style.WindowRounding    = 8.0f;
        style.ChildRounding     = 8.0f;
        style.FrameRounding     = 6.0f;
        style.PopupRounding     = 8.0f;
        style.ScrollbarRounding = 4.0f;
        style.GrabRounding      = 4.0f;
        style.TabRounding       = 6.0f;

        style.WindowPadding     = ImVec2(8, 6);
        style.FramePadding      = ImVec2(8, 5);
        style.ItemSpacing       = ImVec2(8, 6);
        style.ItemInnerSpacing  = ImVec2(6, 4);
        style.WindowBorderSize  = 0.0f;
        style.ChildBorderSize   = 1.0f;
        style.FrameBorderSize   = 0.0f;

        // Audiophile Dark Slate Palette
        ImVec4* colors = style.Colors;
        colors[ImGuiCol_WindowBg]             = ImVec4(0.045f, 0.055f, 0.070f, 1.00f); // #0B0E12
        colors[ImGuiCol_ChildBg]              = CardBg;
        colors[ImGuiCol_PopupBg]              = ImVec4(0.070f, 0.085f, 0.110f, 0.98f);
        colors[ImGuiCol_Border]               = BorderSubtle;
        colors[ImGuiCol_BorderShadow]         = ImVec4(0.000f, 0.000f, 0.000f, 0.00f);

        colors[ImGuiCol_FrameBg]              = ImVec4(0.100f, 0.125f, 0.160f, 1.00f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.140f, 0.175f, 0.220f, 1.00f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.000f, 0.408f, 0.455f, 1.00f);

        colors[ImGuiCol_TitleBg]              = ImVec4(0.045f, 0.055f, 0.070f, 1.00f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.080f, 0.100f, 0.125f, 1.00f);

        colors[ImGuiCol_Button]               = ImVec4(0.120f, 0.150f, 0.190f, 1.00f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.149f, 0.776f, 0.855f, 0.30f);
        colors[ImGuiCol_ButtonActive]         = AccentTeal;

        colors[ImGuiCol_Header]               = ImVec4(0.120f, 0.150f, 0.190f, 1.00f);
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.149f, 0.776f, 0.855f, 0.30f);
        colors[ImGuiCol_HeaderActive]         = AccentTeal;

        colors[ImGuiCol_Tab]                  = ImVec4(0.080f, 0.100f, 0.130f, 1.00f);
        colors[ImGuiCol_TabHovered]           = ImVec4(0.149f, 0.776f, 0.855f, 0.35f);
        colors[ImGuiCol_TabActive]            = ImVec4(0.140f, 0.175f, 0.220f, 1.00f);
        colors[ImGuiCol_TabUnfocused]         = ImVec4(0.060f, 0.075f, 0.095f, 1.00f);
        colors[ImGuiCol_TabUnfocusedActive]  = ImVec4(0.100f, 0.125f, 0.160f, 1.00f);

        colors[ImGuiCol_Text]                 = TextBright;
        colors[ImGuiCol_TextDisabled]         = TextMuted;

        colors[ImGuiCol_PlotLines]            = AccentTeal;
        colors[ImGuiCol_PlotLinesHovered]     = AccentGold;
        colors[ImGuiCol_PlotHistogram]        = AccentTeal;
        colors[ImGuiCol_PlotHistogramHovered] = AccentGold;
    }

    bool renderPillButton(const char* label, bool isSelected, ImVec2 size) {
        if (isSelected) {
            ImGui::PushStyleColor(ImGuiCol_Button, AccentTeal);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.85f, 0.92f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, AccentTeal);
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.04f, 0.06f, 0.08f, 1.0f)); // High-contrast dark text
        } else {
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.10f, 0.13f, 0.17f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.15f, 0.19f, 0.24f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.149f, 0.776f, 0.855f, 0.5f));
            ImGui::PushStyleColor(ImGuiCol_Text, TextMuted);
        }

        bool clicked = ImGui::Button(label, size);
        ImGui::PopStyleColor(4);
        return clicked;
    }

    void renderLevelMeter(const char* label, float value, float maxValue, const char* unitStr, ImVec4 color, float width) {
        float fraction = std::clamp(value / (maxValue > 0.0001f ? maxValue : 1.0f), 0.0f, 1.0f);
        
        char valBuf[32];
        snprintf(valBuf, sizeof(valBuf), "%.3f %s", value, unitStr);

        ImGui::TextColored(TextDim, "%s", label);
        ImGui::SameLine(width - ImGui::CalcTextSize(valBuf).x);
        ImGui::TextColored(TextBright, "%s", valBuf);

        // Segmented LED VU Meter Bar
        const int totalSegments = 20;
        int activeSegments = static_cast<int>(fraction * totalSegments + 0.5f);
        if (activeSegments > totalSegments) activeSegments = totalSegments;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 barPos = ImGui::GetCursorScreenPos();
        float segSpacing = 2.0f;
        float segWidth = (width - (totalSegments - 1) * segSpacing) / static_cast<float>(totalSegments);
        float segHeight = 5.0f;

        for (int i = 0; i < totalSegments; ++i) {
            float x1 = barPos.x + i * (segWidth + segSpacing);
            float y1 = barPos.y;
            float x2 = x1 + segWidth;
            float y2 = y1 + segHeight;

            ImU32 segColor;
            if (i < activeSegments) {
                if (i < 13) {
                    segColor = IM_COL32(0, 230, 118, 255); // Green (Safe)
                } else if (i < 17) {
                    segColor = IM_COL32(255, 215, 0, 255); // Amber (Warning)
                } else {
                    segColor = IM_COL32(255, 82, 82, 255);  // Red (High)
                }
            } else {
                segColor = IM_COL32(25, 32, 42, 180); // Inactive dark LED
            }

            drawList->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), segColor, 1.0f);
        }

        ImGui::Dummy(ImVec2(width, segHeight + 3.0f));
    }
}
