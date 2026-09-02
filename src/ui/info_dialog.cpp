#include "info_dialog.hpp"
#include "../theme.hpp"

InfoDialog::InfoDialog() {}

void InfoDialog::render() {
    if (!m_isOpen) return;

    if (m_shouldOpenPopup) {
        ImGui::OpenPopup("GrooveSpeed Guide & Info");
        m_shouldOpenPopup = false;
    }
    ImGui::SetNextWindowSize(ImVec2(760, 440), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(100, 50), ImGuiCond_Always);

    if (ImGui::BeginPopupModal("GrooveSpeed Guide & Info", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        
        if (ImGui::BeginTabBar("InfoTabs")) {
            
            // Tab 0: PS Vita Hardware Controls & Shortcuts
            if (ImGui::BeginTabItem("PS Vita Controls")) {
                ImGui::TextColored(GrooveTheme::AccentTeal, "PHYSICAL BUTTON SHORTCUTS & TOUCH CONTROLS");
                ImGui::Separator();
                ImGui::Spacing();

                if (ImGui::BeginTable("ControlsTable", 3, ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp)) {
                    ImGui::TableSetupColumn("Button", ImGuiTableColumnFlags_WidthFixed, 150.0f);
                    ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 200.0f);
                    ImGui::TableSetupColumn("Details", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    auto renderRow = [](const char* btn, const ImVec4& color, const char* action, const char* details) {
                        ImGui::TableNextRow();
                        ImGui::TableNextColumn();
                        ImGui::TextColored(color, "%s", btn);
                        ImGui::TableNextColumn();
                        ImGui::TextUnformatted(action);
                        ImGui::TableNextColumn();
                        ImGui::TextColored(GrooveTheme::TextMuted, "%s", details);
                    };

                    renderRow("[ X ] Cross", GrooveTheme::AccentTeal, "Start / Cancel Test", "Starts 3s countdown if flat; prompts calibration if tilted");
                    renderRow("[ (O) ] Circle", GrooveTheme::AccentRed, "Back / Close Dialog", "Dismisses active popups or cancels running measurement");
                    renderRow("[ [ ] ] Square", GrooveTheme::AccentGold, "Toggle Auto-Start", "Arms or disarms automatic trigger on stable RPM");
                    renderRow("[ / \\ ] Triangle", GrooveTheme::AccentGreen, "Calibration Wizard", "Opens or closes the 2-step Gyro / Level calibrator");
                    renderRow("[ D-Pad Left / Right ]", GrooveTheme::AccentTeal, "Cycle Target RPM", "Switches between 33 1/3, 45, and 78 RPM modes");
                    renderRow("[ D-Pad Up / Down ]", GrooveTheme::AccentTeal, "Cycle Duration", "Switches test length (5s, 10s, 15s, 30s)");
                    renderRow("[ L1 / R1 Shoulders ]", GrooveTheme::AccentGold, "Switch Visualizer Tabs", "Cycles Strobe Disc, Live Graph, Polar Plot, History");
                    renderRow("[ START ]", GrooveTheme::AccentGreen, "Quick Start / Stop", "Instantly starts or stops measurement session");
                    renderRow("[ SELECT ]", GrooveTheme::AccentTeal, "Open / Close Manual", "Toggles this Guide & Info dialog");

                    ImGui::EndTable();
                }

                ImGui::Spacing();
                ImGui::TextColored(GrooveTheme::AccentTeal, "TOUCHSCREEN GESTURES");
                ImGui::BulletText("Tap any button, pill, or tab directly on the capacitive screen.");
                ImGui::BulletText("Tap GROOVESPEED title 7 times quickly to reveal the hidden Demo Mode.");
                ImGui::EndTabItem();
            }

            // Tab 1: How to Measure
            if (ImGui::BeginTabItem("Mounting & Setup")) {
                ImGui::TextColored(GrooveTheme::AccentTeal, "TURNTABLE MOUNTING INSTRUCTIONS");
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::BulletText("Place a standard 45 RPM adapter over the center spindle pin.");
                ImGui::BulletText("Place a small non-slip rubber pad or silicone disc on top of the adapter.");
                ImGui::BulletText("Rest your PS Vita FACE UP centered on top of the rubber pad.");
                ImGui::BulletText("Select your target speed (33 1/3, 45, or 78 RPM) and tap Start.");
                ImGui::BulletText("With Auto-Start enabled, the app will start automatically once the platter stabilizes!");
                ImGui::EndTabItem();
            }

            // Tab 2: Metrics Explanation
            if (ImGui::BeginTabItem("Diagnostics Guide")) {
                ImGui::TextColored(GrooveTheme::AccentGold, "UNDERSTANDING YOUR MEASUREMENTS");
                ImGui::Separator();
                ImGui::Spacing();

                ImGui::TextColored(GrooveTheme::AccentTeal, "1. Wow & Flutter (%%)");
                ImGui::Text("Measures speed fluctuations caused by motor drag or belt stretch.");
                ImGui::BulletText("DIN 45507 Mode: Standard weighted filter focusing on 4.0 Hz peak human perception.");
                ImGui::BulletText("Unweighted Mode: Raw peak-to-peak speed variance.");

                ImGui::Spacing();
                ImGui::TextColored(GrooveTheme::AccentTeal, "2. Pitch Deviation (%%)");
                ImGui::Text("Percentage difference between average measured RPM and target speed.");

                ImGui::Spacing();
                ImGui::TextColored(GrooveTheme::AccentTeal, "3. Platter Wobble & Motor Rumble");
                ImGui::BulletText("Wobble: Vertical G-force variance caused by warped platters.");
                ImGui::BulletText("Rumble: High-frequency FFT spectral energy from motor bearings.");
                ImGui::EndTabItem();
            }

            // Tab 3: GrooveList Promotion
            if (ImGui::BeginTabItem("GrooveList Android Companion")) {
                ImGui::TextColored(GrooveTheme::AccentGold, "GROOVELIST VINYL CATALOG");
                ImGui::Separator();
                ImGui::Spacing();
                ImGui::Text("Track, organize, and value your vinyl record collection with GrooveList!");
                ImGui::BulletText("Scan barcodes or search Discogs database.");
                ImGui::BulletText("Keep track of vinyl grading, sleeve condition, and market values.");
                ImGui::BulletText("Sync your diagnostics report cards directly with your turntable setup notes.");
                ImGui::Spacing();
                ImGui::TextColored(GrooveTheme::AccentTeal, "Official Website: smartpickle.me/groovelist");
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        ImGui::Spacing();
        ImGui::Separator();
        if (ImGui::Button("CLOSE GUIDE", ImVec2(720, 35))) {
            close();
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
