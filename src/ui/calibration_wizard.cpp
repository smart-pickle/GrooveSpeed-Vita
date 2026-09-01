#include "calibration_wizard.hpp"
#include "../theme.hpp"
#include "../storage.hpp"
#include <numeric>
#include <cmath>
#include <algorithm>

CalibrationWizard::CalibrationWizard() {}

void CalibrationWizard::open() {
    m_step = WizardStep::STEP1_STATIC_ZEROING;
    m_phase = StepPhase::PREPARE;
    m_progress = 0.0f;
    m_timer = 0.0f;
    m_step1MeasuredBias = 0.0f;
    m_step1Success = false;
    m_step2MeasuredRpm = 0.0f;
    m_step2CalculatedOffset = 0.0f;
    m_step2Success = false;
    m_samples.clear();
    m_shouldOpenPopup = true;
}

void CalibrationWizard::close() {
    m_step = WizardStep::IDLE;
    m_phase = StepPhase::PREPARE;
}

void CalibrationWizard::render(TurntableSensorManager& sensorMgr, float deltaTime) {
    if (m_step == WizardStep::IDLE) return;

    if (m_shouldOpenPopup) {
        ImGui::OpenPopup("Calibration Wizard");
        m_shouldOpenPopup = false;
    }

    ImGui::SetNextWindowSize(ImVec2(600, 390), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(180, 75), ImGuiCond_Always);

    if (ImGui::BeginPopupModal("Calibration Wizard", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove)) {
        
        // -------------------------------------------------------------
        // TOP STEP BREADCRUMBS INDICATOR
        // -------------------------------------------------------------
        bool s1Active = (m_step == WizardStep::STEP1_STATIC_ZEROING);
        bool s2Active = (m_step == WizardStep::STEP2_SPIN_REFERENCE);
        bool s3Active = (m_step == WizardStep::SUMMARY);

        ImGui::TextColored(s1Active ? GrooveTheme::AccentTeal : (m_step1Success ? GrooveTheme::AccentGreen : GrooveTheme::TextMuted), 
            "[ 1. ZERO GYRO ]");
        ImGui::SameLine();
        ImGui::TextColored(GrooveTheme::TextDim, "----");
        ImGui::SameLine();
        ImGui::TextColored(s2Active ? GrooveTheme::AccentTeal : (m_step2Success ? GrooveTheme::AccentGreen : GrooveTheme::TextMuted), 
            "[ 2. SPIN RPM (OPTIONAL) ]");
        ImGui::SameLine();
        ImGui::TextColored(GrooveTheme::TextDim, "----");
        ImGui::SameLine();
        ImGui::TextColored(s3Active ? GrooveTheme::AccentTeal : GrooveTheme::TextMuted, 
            "[ 3. SUMMARY ]");

        ImGui::Separator();
        ImGui::Spacing();

        SensorData data = sensorMgr.getData();

        // -------------------------------------------------------------
        // STEP 1: STATIC GYRO ZEROING
        // -------------------------------------------------------------
        if (m_step == WizardStep::STEP1_STATIC_ZEROING) {
            ImGui::TextColored(GrooveTheme::AccentTeal, "STEP 1: Static Gyro Bias Zeroing (Essential)");
            ImGui::Text("1. Place your PS Vita completely FLAT on any stationary table or platter.");
            ImGui::Text("2. Ensure the device is not touched, moved, or vibrating.");
            ImGui::Spacing();

            // Real-time Flatness & Angular Noise Card
            ImGui::PushStyleColor(ImGuiCol_ChildBg, GrooveTheme::CardElevated);
            ImGui::BeginChild("S1Card", ImVec2(0, 52), true, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::Text("Device Orientation: ");
                ImGui::SameLine();
                if (data.isFlat) {
                    ImGui::TextColored(GrooveTheme::AccentGreen, "[ LEVEL / STATIONARY ]");
                } else {
                    ImGui::TextColored(GrooveTheme::AccentRed, "[ TILTED / CHECK PLACEMENT ]");
                }
                ImGui::TextColored(GrooveTheme::TextDim, "Active Gyro Bias: %+.4f rad/s", sensorMgr.getGyroZeroBias());
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();

            ImGui::Spacing();

            // Sampling Phase Handling
            if (m_phase == StepPhase::PREPARE) {
                if (ImGui::Button("START ZEROING SAMPLES (3 SECONDS)", ImVec2(-1, 38))) {
                    m_phase = StepPhase::SAMPLING;
                    m_timer = 0.0f;
                    m_progress = 0.0f;
                }
                ImGui::TextColored(GrooveTheme::TextMuted, "Tap button above once the device is stationary and level.");
            }
            else if (m_phase == StepPhase::SAMPLING) {
                m_timer += deltaTime;
                float duration = 3.0f;
                m_progress = std::min(m_timer / duration, 1.0f);

                ImGui::ProgressBar(m_progress, ImVec2(-1, 32), "SAMPLING STATIC GYRO...");

                if (m_timer >= duration) {
                    sensorMgr.calibrateZeroGyro();
                    m_step1MeasuredBias = sensorMgr.getGyroZeroBias();
                    m_step1Success = true;
                    m_phase = StepPhase::COMPLETED;
                    sensorMgr.triggerHapticVibration(0, 150);
                }
            }
            else if (m_phase == StepPhase::COMPLETED) {
                ImGui::TextColored(GrooveTheme::AccentGreen, "[OK] STATIC GYRO BIAS ACQUIRED: %+.4f rad/s", m_step1MeasuredBias);
                ImGui::TextColored(GrooveTheme::TextBright, "Gyro calibrated with factory hardware precision.");
                ImGui::TextColored(GrooveTheme::TextDim, "Tap FINISH to save and start measuring, or ADVANCED to trim spin reference.");
                ImGui::Dummy(ImVec2(0, 4));
            }

            // Bottom Navigation Bar
            ImGui::SetCursorPosY(340);
            ImGui::Separator();

            if (ImGui::Button("RESET STEP", ImVec2(100, 32))) {
                sensorMgr.setGyroZeroBias(0.0f);
                m_step1MeasuredBias = 0.0f;
                m_step1Success = false;
                m_phase = StepPhase::PREPARE;
                m_timer = 0.0f;
                m_progress = 0.0f;
            }

            ImGui::SameLine(115);
            if (ImGui::Button("CANCEL", ImVec2(80, 32))) {
                close();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine(210);
            if (m_step1Success) {
                // Recommended Primary Action: Finish & Save directly!
                ImGui::PushStyleColor(ImGuiCol_Button, GrooveTheme::AccentGreen);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.05f, 0.08f, 0.10f, 1.0f));
                if (ImGui::Button("FINISH & SAVE (RECOMMENDED)", ImVec2(225, 32))) {
                    StorageManager::saveCalibration({sensorMgr.getGyroZeroBias(), sensorMgr.getCalibrationOffset()});
                    close();
                    ImGui::CloseCurrentPopup();
                }
                ImGui::PopStyleColor(2);

                ImGui::SameLine(445);
                if (ImGui::Button("ADVANCED >>", ImVec2(130, 32))) {
                    m_step = WizardStep::STEP2_SPIN_REFERENCE;
                    m_phase = StepPhase::PREPARE;
                    m_timer = 0.0f;
                    m_progress = 0.0f;
                    m_samples.clear();
                }
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.19f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, GrooveTheme::TextDim);
                ImGui::Button("FINISH & SAVE", ImVec2(225, 32));
                ImGui::SameLine(445);
                ImGui::Button("ADVANCED >>", ImVec2(130, 32));
                ImGui::PopStyleColor(2);
            }
        }
        // -------------------------------------------------------------
        // STEP 2: SPIN REFERENCE CALIBRATION (OPTIONAL)
        // -------------------------------------------------------------
        else if (m_step == WizardStep::STEP2_SPIN_REFERENCE) {
            ImGui::TextColored(GrooveTheme::AccentGold, "STEP 2 (OPTIONAL): Reference Turntable Spin Trim");
            ImGui::TextColored(GrooveTheme::TextDim, "Use only if you own a quartz-locked deck (e.g. Technics) or laser tachometer.");
            ImGui::Text("Place PS Vita on reference platter spinning at chosen speed:");
            ImGui::Spacing();

            // Speed Selector Pills
            bool is33 = std::abs(m_refTargetSpeed - 33.33f) < 0.1f;
            bool is45 = std::abs(m_refTargetSpeed - 45.0f) < 0.1f;
            if (GrooveTheme::renderPillButton("33 1/3 RPM", is33, ImVec2(110, 24))) {
                m_refTargetSpeed = 33.3333f;
            }
            ImGui::SameLine();
            if (GrooveTheme::renderPillButton("45 RPM", is45, ImVec2(110, 24))) {
                m_refTargetSpeed = 45.0f;
            }

            ImGui::Spacing();

            // Real-time Speed Preview Card
            ImGui::PushStyleColor(ImGuiCol_ChildBg, GrooveTheme::CardElevated);
            ImGui::BeginChild("S2Card", ImVec2(0, 48), true, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::Text("Live Measured Speed: ");
                ImGui::SameLine();
                ImGui::TextColored(GrooveTheme::AccentTeal, "%.2f RPM", data.rpm);
                ImGui::SameLine(280);
                ImGui::TextColored(GrooveTheme::TextMuted, "Target Reference: %.2f RPM", m_refTargetSpeed);
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();

            ImGui::Spacing();

            // Sampling Phase Handling
            if (m_phase == StepPhase::PREPARE) {
                if (ImGui::Button("START SPIN SPEED SAMPLING (5 SECONDS)", ImVec2(-1, 38))) {
                    m_phase = StepPhase::SAMPLING;
                    m_timer = 0.0f;
                    m_progress = 0.0f;
                    m_samples.clear();
                }
                ImGui::TextColored(GrooveTheme::TextMuted, "Tap above once your turntable motor has reached full speed.");
            }
            else if (m_phase == StepPhase::SAMPLING) {
                m_timer += deltaTime;
                float duration = 5.0f;
                m_progress = std::min(m_timer / duration, 1.0f);

                m_samples.push_back(data.rawRpm);

                ImGui::ProgressBar(m_progress, ImVec2(-1, 32), "SAMPLING PLATTER ROTATION...");

                if (m_timer >= duration) {
                    float avgRaw = std::accumulate(m_samples.begin(), m_samples.end(), 0.0f) / 
                                   static_cast<float>(std::max<size_t>(m_samples.size(), 1));
                    float offset = avgRaw - m_refTargetSpeed;
                    sensorMgr.setCalibrationOffset(-offset);
                    m_step2MeasuredRpm = avgRaw;
                    m_step2CalculatedOffset = -offset;
                    m_step2Success = true;
                    m_phase = StepPhase::COMPLETED;
                    sensorMgr.triggerHapticVibration(200, 200);
                }
            }
            else if (m_phase == StepPhase::COMPLETED) {
                ImGui::TextColored(GrooveTheme::AccentGreen, "[OK] MEASURED: %.2f RPM  |  OFFSET: %+.3f RPM", 
                    m_step2MeasuredRpm, m_step2CalculatedOffset);
                ImGui::TextColored(GrooveTheme::TextBright, "Spin calibration complete. Tap NEXT STEP to review and save.");
                ImGui::Dummy(ImVec2(0, 6));
            }

            // Bottom Navigation Bar
            ImGui::SetCursorPosY(340);
            ImGui::Separator();

            if (ImGui::Button("<< BACK", ImVec2(80, 32))) {
                m_step = WizardStep::STEP1_STATIC_ZEROING;
                m_phase = StepPhase::COMPLETED;
            }

            ImGui::SameLine(90);
            if (ImGui::Button("RESET STEP", ImVec2(100, 32))) {
                sensorMgr.setCalibrationOffset(0.0f);
                m_step2MeasuredRpm = 0.0f;
                m_step2CalculatedOffset = 0.0f;
                m_step2Success = false;
                m_phase = StepPhase::PREPARE;
                m_timer = 0.0f;
                m_progress = 0.0f;
                m_samples.clear();
            }

            ImGui::SameLine(200);
            if (ImGui::Button("SKIP & FINISH", ImVec2(120, 32))) {
                StorageManager::saveCalibration({sensorMgr.getGyroZeroBias(), sensorMgr.getCalibrationOffset()});
                close();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine(330);
            if (ImGui::Button("CANCEL", ImVec2(90, 32))) {
                close();
                ImGui::CloseCurrentPopup();
            }

            ImGui::SameLine(430);
            if (!m_step2Success) {
                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.12f, 0.15f, 0.19f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_Text, GrooveTheme::TextDim);
                ImGui::Button("NEXT STEP >>", ImVec2(145, 32));
                ImGui::PopStyleColor(2);
            } else {
                if (ImGui::Button("NEXT STEP >>", ImVec2(145, 32))) {
                    m_step = WizardStep::SUMMARY;
                }
            }
        }
        // -------------------------------------------------------------
        // STEP 3: SUMMARY & FINE ADJUSTMENT
        // -------------------------------------------------------------
        else if (m_step == WizardStep::SUMMARY) {
            ImGui::TextColored(GrooveTheme::AccentTeal, "CALIBRATION SUMMARY & ACTIVE OFFSETS");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_ChildBg, GrooveTheme::CardElevated);
            ImGui::BeginChild("SummaryCard", ImVec2(0, 95), true, ImGuiWindowFlags_NoScrollbar);
            {
                ImGui::Text("Static Gyro Bias:          ");
                ImGui::SameLine();
                ImGui::TextColored(GrooveTheme::AccentGreen, "%+.4f rad/s", sensorMgr.getGyroZeroBias());

                ImGui::Text("RPM Calibration Offset:    ");
                ImGui::SameLine();
                ImGui::TextColored(GrooveTheme::AccentTeal, "%+.3f RPM", sensorMgr.getCalibrationOffset());

                ImGui::Spacing();
                ImGui::TextColored(GrooveTheme::TextMuted, "Manual Fine Adjustment:");
                ImGui::SameLine();
                if (ImGui::Button("- 0.05")) {
                    sensorMgr.setCalibrationOffset(sensorMgr.getCalibrationOffset() - 0.05f);
                }
                ImGui::SameLine();
                if (ImGui::Button("- 0.01")) {
                    sensorMgr.setCalibrationOffset(sensorMgr.getCalibrationOffset() - 0.01f);
                }
                ImGui::SameLine();
                if (ImGui::Button("+ 0.01")) {
                    sensorMgr.setCalibrationOffset(sensorMgr.getCalibrationOffset() + 0.01f);
                }
                ImGui::SameLine();
                if (ImGui::Button("+ 0.05")) {
                    sensorMgr.setCalibrationOffset(sensorMgr.getCalibrationOffset() + 0.05f);
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();

            ImGui::Spacing();
            ImGui::TextColored(GrooveTheme::TextBright, "All offsets will be applied in real time and saved to ux0:data/groovespeed/config.txt");

            // Bottom Navigation Bar
            ImGui::SetCursorPosY(340);
            ImGui::Separator();

            if (ImGui::Button("RESET TO FACTORY DEFAULTS", ImVec2(210, 32))) {
                sensorMgr.resetCalibration();
                m_step1MeasuredBias = 0.0f;
                m_step1Success = false;
                m_step2MeasuredRpm = 0.0f;
                m_step2CalculatedOffset = 0.0f;
                m_step2Success = false;
            }

            ImGui::SameLine(270);
            if (ImGui::Button("RE-RUN WIZARD", ImVec2(130, 32))) {
                m_step = WizardStep::STEP1_STATIC_ZEROING;
                m_phase = StepPhase::PREPARE;
                m_timer = 0.0f;
                m_progress = 0.0f;
            }

            ImGui::SameLine(425);
            if (ImGui::Button("SAVE & CLOSE", ImVec2(150, 32))) {
                StorageManager::saveCalibration({sensorMgr.getGyroZeroBias(), sensorMgr.getCalibrationOffset()});
                close();
                ImGui::CloseCurrentPopup();
            }
        }

        ImGui::EndPopup();
    }
}
