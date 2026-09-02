#include <SDL.h>
#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

#include "sensor_manager.hpp"
#include "dsp_engine.hpp"
#include "theme.hpp"
#include "font_manager.hpp"
#include "ui/strobe_ring.hpp"
#include "ui/rpm_graph.hpp"
#include "ui/polar_plot.hpp"
#include "ui/calibration_wizard.hpp"
#include "ui/info_dialog.hpp"
#include "storage.hpp"

#include <vector>
#include <chrono>
#include <string>
#include <algorithm>
#include <psp2/io/stat.h>

// PS Vita OS Process Configuration: Set 2MB Main Thread Stack Size & 64MB Heap
extern "C" {
    unsigned int sceUserMainThreadStackSizeInBytes = 2 * 1024 * 1024;
    unsigned int sceUserMainThreadAttribute = 0;
    unsigned int sceLibcHeapSize = 64 * 1024 * 1024; // 64 MB User Heap
}

void logBoot(const char* msg, bool overwrite = false) {
    sceIoMkdir("ux0:data", 0777);
    sceIoMkdir("ux0:data/groovespeed", 0777);
    FILE* f = fopen("ux0:/data/groovespeed/boot_log.txt", overwrite ? "w" : "a");
    if (f) {
        fprintf(f, "%s\n", msg);
        fclose(f);
    }
}

int main(int argc, char* argv[]) {
    logBoot("[GrooveSpeed] App starting main()", true);

    // Set touch & mouse interaction hints for PS Vita
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "1");
    SDL_SetHint(SDL_HINT_MOUSE_TOUCH_EVENTS, "1");

    // 1. Initialize SDL2 Subsystems
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) != 0) {
        char errBuf[256];
        snprintf(errBuf, sizeof(errBuf), "[GrooveSpeed] SDL_Init failed: %s", SDL_GetError());
        logBoot(errBuf);
        return -1;
    }
    logBoot("[GrooveSpeed] SDL_Init success.");

    SDL_Window* window = SDL_CreateWindow(
        "GrooveSpeed Vita",
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        960, 544,
        0
    );
    if (!window) {
        char errBuf[256];
        snprintf(errBuf, sizeof(errBuf), "[GrooveSpeed] SDL_CreateWindow failed: %s", SDL_GetError());
        logBoot(errBuf);
        return -1;
    }
    logBoot("[GrooveSpeed] SDL_CreateWindow success.");

    SDL_Renderer* renderer = SDL_CreateRenderer(
        window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC
    );
    if (!renderer) {
        char errBuf[256];
        snprintf(errBuf, sizeof(errBuf), "[GrooveSpeed] SDL_CreateRenderer failed: %s", SDL_GetError());
        logBoot(errBuf);
        return -1;
    }
    logBoot("[GrooveSpeed] SDL_CreateRenderer success.");

    // 2. Initialize ImGui using standard SDL_Renderer backend
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr; // Prevent read-only app0:/imgui.ini write crash
    GrooveFonts::initFonts(io);
    logBoot("[GrooveSpeed] Custom TrueType fonts initialized.");

    GrooveTheme::applyTheme();
    logBoot("[GrooveSpeed] Theme applied.");

    // Note: Use Manual Gamepad Mode to prevent SDL_GameControllerMapping crash on physical PS Vita
    ImGui_ImplSDL2_InitForSDLRenderer(window, renderer);
    ImGui_ImplSDL2_SetGamepadMode(ImGui_ImplSDL2_GamepadMode_Manual, nullptr, 0);
    ImGui_ImplSDLRenderer2_Init(renderer);
    logBoot("[GrooveSpeed] ImGui backends initialized.");

    // Ensure data directories exist on ux0:
    StorageManager::ensureDataDirectory();
    StorageManager::ensurePictureDirectory();
    logBoot("[GrooveSpeed] Storage directories checked.");

    // 4. Initialize Hardware & Settings
    TurntableSensorManager sensorMgr;
    sensorMgr.init();
    logBoot("[GrooveSpeed] Sensor manager initialized.");

    CalibrationConfig savedCal = StorageManager::loadCalibration();
    sensorMgr.setGyroZeroBias(savedCal.gyroZeroBias);
    sensorMgr.setCalibrationOffset(savedCal.calibrationOffset);
    logBoot("[GrooveSpeed] Calibration loaded.");

    DspEngine dsp;
    CalibrationWizard wizard;
    InfoDialog infoDlg;

    // Session History Cache
    std::vector<SessionRecord> cachedHistory = StorageManager::loadHistory();
    bool historyNeedsReload = false;
    logBoot("[GrooveSpeed] History loaded.");

    // History Deletion & Undo State
    SessionRecord lastDeletedRecord{};
    bool hasDeletedItem = false;

    // Measurement State Machine
    float targetSpeed = 33.33f;
    WowWeighting weighting = WowWeighting::DIN;
    int durationSeconds = 10;

    bool autoStartEnabled = true;
    bool isAutoStartArmed = true;
    bool isWaitingForStability = false;
    std::vector<float> autoStartWindow;

    int countdownSeconds = 0;
    float countdownTimer = 0.0f;

    bool isMeasuring = false;
    bool sessionFinished = false;
    SessionRecord lastReport{};

    std::vector<float> rpmWaveformHistory;
    rpmWaveformHistory.reserve(200);

    int64_t sessionStartTimestamp = 0;
    auto lastTime = std::chrono::high_resolution_clock::now();

    // 7-tap Easter egg state for revealing Demo Mode
    int titleTapCount = 0;
    uint32_t lastTitleTapTick = 0;
    bool showDemoButton = false;
    float demoUnlockedToastTimer = 0.0f;

    // Reset mouse position to un-hovered state before frame 1 to prevent HoveredWindow NULL dereference
    io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
    printf("[GrooveSpeed] Entering main render loop...\n"); fflush(stdout);

    // 5. Main Event & Render Loop
    bool running = true;
    int frameNum = 0;
    while (running) {
        frameNum++;
        if (frameNum <= 5) {
            printf("[GrooveSpeed] Start Frame %d\n", frameNum); fflush(stdout);
        }
        auto currentTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
        lastTime = currentTime;

        int64_t currentTimestampMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            currentTime.time_since_epoch()
        ).count();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT) running = false;
        }

        sensorMgr.setDemoTargetRpm(targetSpeed);
        sensorMgr.update(deltaTime);
        SensorData data = sensorMgr.getData();

        if (rpmWaveformHistory.size() >= 200) {
            rpmWaveformHistory.erase(rpmWaveformHistory.begin());
        }
        rpmWaveformHistory.push_back(data.rpm);

        // Auto-Start Monitor
        if (autoStartEnabled && !isMeasuring && countdownSeconds == 0 && data.isFlat) {
            if (data.instantRpm > 10.0f) {
                if (isAutoStartArmed) {
                    isWaitingForStability = true;
                    autoStartWindow.push_back(data.instantRpm);

                    if (autoStartWindow.size() > 20) {
                        autoStartWindow.erase(autoStartWindow.begin());
                        auto [minIt, maxIt] = std::minmax_element(autoStartWindow.begin(), autoStartWindow.end());
                        float range = *maxIt - *minIt;
                        float avg = (data.instantRpm + *minIt + *maxIt) / 3.0f;

                        bool isNearTarget = std::abs(avg - targetSpeed) < (targetSpeed * 0.05f);
                        bool isStable = range < 0.15f;

                        if (isNearTarget && isStable) {
                            autoStartWindow.clear();
                            isWaitingForStability = false;
                            isAutoStartArmed = false;

                            countdownSeconds = 3;
                            countdownTimer = 0.0f;
                            sensorMgr.triggerHapticVibration(100, 100);
                        }
                    }
                }
            } else {
                autoStartWindow.clear();
                isWaitingForStability = false;
                if (data.instantRpm < 5.0f) {
                    isAutoStartArmed = true;
                }
            }
        }

        // 3-Second Countdown Handler
        if (countdownSeconds > 0) {
            countdownTimer += deltaTime;
            if (countdownTimer >= 1.0f) {
                countdownTimer -= 1.0f;
                countdownSeconds--;
                sensorMgr.triggerHapticVibration(120, 120);

                if (countdownSeconds == 0) {
                    dsp.resetSession(targetSpeed, weighting, durationSeconds);
                    sessionStartTimestamp = currentTimestampMs;
                    isMeasuring = true;
                    sessionFinished = false;
                    sensorMgr.triggerHapticVibration(255, 255);
                }
            }
        }

        // Session Measurement Active Update
        if (isMeasuring) {
            int64_t relativeMs = currentTimestampMs - sessionStartTimestamp;
            dsp.addSample(data.rawRpm, data.currentOrientation, data.platterWobble, data.motorRumble, relativeMs);
            
            if (dsp.isSessionComplete()) {
                isMeasuring = false;
                sessionFinished = true;
                lastReport = dsp.computeFinalReport();

                StorageManager::saveSession(lastReport);
                historyNeedsReload = true;
                sensorMgr.triggerHapticVibration(200, 200);
            }
        }

        // Reload history cache if requested
        if (historyNeedsReload) {
            cachedHistory = StorageManager::loadHistory();
            historyNeedsReload = false;
        }

        // 6. Render ImGui Frame (CRITICAL ORDER: SDLRenderer2 NewFrame -> SDL2 NewFrame -> NewFrame)
        ImGui_ImplSDLRenderer2_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(960, 544));
        ImGui::Begin("GrooveSpeed", nullptr,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus
        );

        // -------------------------------------------------------------
        // TOP NAVIGATION HEADER (Height ~28px)
        // -------------------------------------------------------------
        if (demoUnlockedToastTimer > 0.0f) {
            demoUnlockedToastTimer -= deltaTime;
        }

        ImVec2 titlePos = ImGui::GetCursorPos();
        if (GrooveFonts::FontBold) ImGui::PushFont(GrooveFonts::FontBold);
        ImGui::TextColored(GrooveTheme::AccentTeal, "GROOVESPEED");
        ImGui::SameLine();
        ImGui::TextColored(GrooveTheme::TextMuted, "| TURNTABLE TESTER");
        if (GrooveFonts::FontBold) ImGui::PopFont();

        // Invisible touch target over GROOVESPEED title (7 taps unlock Demo Mode)
        ImVec2 postTitlePos = ImGui::GetCursorPos();
        ImGui::SetCursorPos(titlePos);
        if (ImGui::InvisibleButton("##title_7tap", ImVec2(240, 24))) {
            uint32_t nowTick = SDL_GetTicks();
            if (nowTick - lastTitleTapTick > 2000) {
                // If more than 2.0s between taps, restart count
                titleTapCount = 1;
            } else {
                titleTapCount++;
            }
            lastTitleTapTick = nowTick;

            if (titleTapCount >= 7) {
                showDemoButton = true;
                demoUnlockedToastTimer = 3.5f;
                sensorMgr.triggerHapticVibration(150, 200);
                titleTapCount = 0;
            }
        }
        ImGui::SetCursorPos(postTitlePos);

        ImGui::SameLine(340);

        // Real-time Flatness Indicator Badge or Demo Unlocked Toast
        if (demoUnlockedToastTimer > 0.0f) {
            ImGui::TextColored(GrooveTheme::AccentTeal, "[ DEMO UNLOCKED ]");
        } else if (data.isFlat) {
            ImGui::TextColored(GrooveTheme::AccentGreen, "[ LEVEL ]");
        } else {
            ImGui::TextColored(GrooveTheme::AccentRed, "[ TILTED ]");
        }

        // Right side quick action pills (DEMO button only visible after 7 taps)
        if (showDemoButton) {
            ImGui::SameLine(625);
            if (GrooveTheme::renderPillButton(sensorMgr.isDemoMode() ? "DEMO ON" : "DEMO OFF", sensorMgr.isDemoMode(), ImVec2(95, 22))) {
                sensorMgr.setDemoMode(!sensorMgr.isDemoMode());
            }
            ImGui::SameLine();
            if (ImGui::Button("CALIBRATE", ImVec2(100, 22))) {
                wizard.open();
            }
            ImGui::SameLine();
            if (ImGui::Button("MANUAL", ImVec2(85, 22))) {
                infoDlg.open();
            }
        } else {
            ImGui::SameLine(730);
            if (ImGui::Button("CALIBRATE", ImVec2(100, 22))) {
                wizard.open();
            }
            ImGui::SameLine();
            if (ImGui::Button("MANUAL", ImVec2(85, 22))) {
                infoDlg.open();
            }
        }

        ImGui::Separator();

        // -------------------------------------------------------------
        // LEFT CONTROL & TELEMETRY PANEL (330px x 485px, ZERO SCROLL)
        // -------------------------------------------------------------
        ImGui::BeginChild("LeftPanel", ImVec2(330, 485), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        {
            // 1. HERO RPM & TELEMETRY CARD
            ImGui::PushStyleColor(ImGuiCol_ChildBg, GrooveTheme::CardElevated);
            ImGui::BeginChild("HeroRpmCard", ImVec2(0, 140), true, ImGuiWindowFlags_NoScrollbar);
            {
                float speedDev = ((data.rpm - targetSpeed) / targetSpeed) * 100.0f;
                bool isLocked = std::abs(speedDev) < 0.15f && data.rpm > 10.0f;

                ImGui::TextColored(GrooveTheme::TextMuted, "LIVE RPM READOUT");
                ImGui::SameLine(ImGui::GetWindowWidth() - 80);
                if (isLocked) {
                    ImGui::TextColored(GrooveTheme::AccentGreen, "LOCKED");
                } else if (data.rpm > 10.0f) {
                    ImGui::TextColored(GrooveTheme::AccentGold, "DRIFT");
                } else {
                    ImGui::TextColored(GrooveTheme::TextDim, "STOPPED");
                }

                // Large 36px Hero RPM Readout
                char rpmStr[32];
                snprintf(rpmStr, sizeof(rpmStr), "%.2f", data.rpm);
                if (GrooveFonts::FontHeroRpm) ImGui::PushFont(GrooveFonts::FontHeroRpm);
                ImGui::TextColored(GrooveTheme::AccentTeal, "%s", rpmStr);
                if (GrooveFonts::FontHeroRpm) ImGui::PopFont();

                ImGui::SameLine();
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
                ImGui::TextColored(GrooveTheme::TextMuted, "RPM");

                ImGui::SameLine(180);
                ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10);
                if (data.rpm > 10.0f) {
                    ImGui::TextColored(isLocked ? GrooveTheme::AccentGreen : GrooveTheme::AccentGold,
                        "Dev: %+.2f%%", speedDev);
                } else {
                    ImGui::TextColored(GrooveTheme::TextDim, "Dev: 0.00%%");
                }

                // Telemetry mini-meters
                GrooveTheme::renderLevelMeter("WOBBLE", data.platterWobble, 0.05f, "g", GrooveTheme::AccentTeal, 290.0f);
                GrooveTheme::renderLevelMeter("RUMBLE", data.motorRumble, 0.02f, "g", GrooveTheme::AccentGold, 290.0f);
            }
            ImGui::EndChild();
            ImGui::PopStyleColor();

            ImGui::Spacing();

            // 2. SPEED SELECTOR PILLS
            ImGui::TextColored(GrooveTheme::TextMuted, "TARGET SPEED");
            if (GrooveTheme::renderPillButton("33 1/3", std::abs(targetSpeed - 33.33f) < 0.1f, ImVec2(98, 28))) targetSpeed = 33.33f;
            ImGui::SameLine();
            if (GrooveTheme::renderPillButton("45 RPM", std::abs(targetSpeed - 45.0f) < 0.1f, ImVec2(98, 28))) targetSpeed = 45.0f;
            ImGui::SameLine();
            if (GrooveTheme::renderPillButton("78 RPM", std::abs(targetSpeed - 78.0f) < 0.1f, ImVec2(98, 28))) targetSpeed = 78.0f;

            ImGui::Spacing();

            // 3. DURATION PILLS
            ImGui::TextColored(GrooveTheme::TextMuted, "DURATION");
            if (GrooveTheme::renderPillButton("5s", durationSeconds == 5, ImVec2(72, 24))) durationSeconds = 5;
            ImGui::SameLine();
            if (GrooveTheme::renderPillButton("10s", durationSeconds == 10, ImVec2(72, 24))) durationSeconds = 10;
            ImGui::SameLine();
            if (GrooveTheme::renderPillButton("15s", durationSeconds == 15, ImVec2(72, 24))) durationSeconds = 15;
            ImGui::SameLine();
            if (GrooveTheme::renderPillButton("30s", durationSeconds == 30, ImVec2(72, 24))) durationSeconds = 30;

            ImGui::Spacing();

            // 4. WOW & FLUTTER WEIGHTING & AUTO-START
            ImGui::TextColored(GrooveTheme::TextMuted, "WEIGHTING & TRIGGER");
            if (GrooveTheme::renderPillButton("DIN 45507", weighting == WowWeighting::DIN, ImVec2(148, 24))) weighting = WowWeighting::DIN;
            ImGui::SameLine();
            if (GrooveTheme::renderPillButton("UNWEIGHTED", weighting == WowWeighting::UNWEIGHTED, ImVec2(148, 24))) weighting = WowWeighting::UNWEIGHTED;

            ImGui::Spacing();
            ImGui::Checkbox("Auto-Start on Stable Speed", &autoStartEnabled);
            if (isWaitingForStability) {
                ImGui::SameLine();
                ImGui::TextColored(GrooveTheme::AccentGold, "(Arming...)");
            }

            ImGui::Spacing();
            ImGui::Separator();
            ImGui::Spacing();

            // 5. PRIMARY ACTION BUTTON / COUNTDOWN / PROGRESS
            if (countdownSeconds > 0) {
                ImGui::PushStyleColor(ImGuiCol_Button, GrooveTheme::AccentGold);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, GrooveTheme::AccentGold);
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, GrooveTheme::AccentGold);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.04f, 0.06f, 0.08f, 1.0f));
                char countBuf[32];
                snprintf(countBuf, sizeof(countBuf), "STARTING IN %ds...", countdownSeconds);
                ImGui::Button(countBuf, ImVec2(-1, 44));
                ImGui::PopStyleColor(4);
            } else if (isMeasuring) {
                ImGui::PushStyleColor(ImGuiCol_Button, GrooveTheme::AccentRed);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.0f, 0.35f, 0.30f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, GrooveTheme::AccentRed);
                if (ImGui::Button("CANCEL MEASUREMENT", ImVec2(-1, 36))) {
                    isMeasuring = false;
                }
                ImGui::PopStyleColor(3);

                // Progress Bar
                float progress = std::clamp((float)dsp.getSamples().size() / (float)(durationSeconds * 100), 0.0f, 1.0f);
                ImGui::PushStyleColor(ImGuiCol_PlotHistogram, GrooveTheme::AccentTeal);
                ImGui::ProgressBar(progress, ImVec2(-1, 6), "");
                ImGui::PopStyleColor();
            } else {
                ImGui::PushStyleColor(ImGuiCol_Button, GrooveTheme::AccentTeal);
                ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.20f, 0.85f, 0.92f, 1.0f));
                ImGui::PushStyleColor(ImGuiCol_ButtonActive, GrooveTheme::AccentTeal);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.04f, 0.06f, 0.08f, 1.0f));
                if (ImGui::Button("START MEASUREMENT", ImVec2(-1, 44))) {
                    dsp.resetSession(targetSpeed, weighting, durationSeconds);
                    sessionStartTimestamp = currentTimestampMs;
                    isMeasuring = true;
                    sessionFinished = false;
                    sensorMgr.triggerHapticVibration(200, 200);
                }
                ImGui::PopStyleColor(4);
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        // -------------------------------------------------------------
        // RIGHT VISUALIZER & HISTORY PANEL (595px x 485px)
        // -------------------------------------------------------------
        ImGui::BeginChild("RightPanel", ImVec2(595, 485), true);
        {
            if (ImGui::BeginTabBar("VisualizerTabs")) {
                if (ImGui::BeginTabItem("  Strobe Disc  ")) {
                    GrooveUI::renderStrobeRing(
                        ImGui::GetWindowDrawList(),
                        ImVec2(ImGui::GetCursorScreenPos().x + 290, ImGui::GetCursorScreenPos().y + 190),
                        160.0f,
                        targetSpeed,
                        data.rpm,
                        data.currentOrientation
                    );
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("  Live RPM Graph  ")) {
                    GrooveUI::renderRpmGraph(
                        ImGui::GetWindowDrawList(),
                        ImGui::GetCursorScreenPos(),
                        ImVec2(575, 380),
                        rpmWaveformHistory,
                        targetSpeed
                    );
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("  Polar Plot  ")) {
                    GrooveUI::renderPolarPlot(
                        ImGui::GetWindowDrawList(),
                        ImVec2(ImGui::GetCursorScreenPos().x + 290, ImGui::GetCursorScreenPos().y + 190),
                        160.0f,
                        dsp.getSamples(),
                        targetSpeed
                    );
                    ImGui::EndTabItem();
                }

                if (ImGui::BeginTabItem("  Session History  ")) {
                    if (hasDeletedItem) {
                        ImGui::TextColored(GrooveTheme::AccentGold, "Item deleted.");
                        ImGui::SameLine();
                        if (ImGui::Button("UNDO")) {
                            StorageManager::saveSession(lastDeletedRecord);
                            hasDeletedItem = false;
                            historyNeedsReload = true;
                        }
                        ImGui::Separator();
                    }

                    if (cachedHistory.empty()) {
                        ImGui::Spacing();
                        ImGui::TextColored(GrooveTheme::TextMuted, "No saved measurement sessions yet.");
                        ImGui::TextColored(GrooveTheme::TextDim, "Run a test and results will be saved here automatically.");
                    } else {
                        if (ImGui::Button("EXPORT ALL TO CSV")) {
                            StorageManager::exportSamplesToCsv(dsp.getSamples(), targetSpeed);
                        }
                        ImGui::Separator();

                        for (int i = 0; i < (int)cachedHistory.size(); ++i) {
                            const auto& rec = cachedHistory[i];
                            ImGui::PushID(i);
                            ImGui::Text("[%lld ms] Target: %.1f | Avg: %.2f RPM | W&F: %.3f%%",
                                static_cast<long long>(rec.timestampMs), rec.targetSpeed, rec.averageRpm, rec.wowPercent);
                            
                            ImGui::SameLine(ImGui::GetWindowWidth() - 140);
                            if (ImGui::Button("REPORT")) {
                                StorageManager::exportReportCardPng(rec);
                            }
                            ImGui::SameLine();
                            if (ImGui::Button("DELETE")) {
                                lastDeletedRecord = rec;
                                hasDeletedItem = true;
                                StorageManager::deleteHistoryRecord(i);
                                historyNeedsReload = true;
                            }
                            ImGui::PopID();
                        }
                    }
                    ImGui::EndTabItem();
                }

                ImGui::EndTabBar();
            }

            // Sleek Measurement Complete Overlay Card
            if (sessionFinished) {
                ImGui::SetCursorPosY(ImGui::GetWindowHeight() - 75);
                ImGui::PushStyleColor(ImGuiCol_ChildBg, GrooveTheme::CardElevated);
                ImGui::BeginChild("SessionSummaryCard", ImVec2(0, 68), true);
                {
                    ImGui::TextColored(GrooveTheme::AccentGreen, "DIAGNOSTIC COMPLETE");
                    ImGui::SameLine(200);
                    if (ImGui::Button("SAVE REPORT PNG", ImVec2(160, 22))) {
                        StorageManager::exportReportCardPng(lastReport);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("EXPORT CSV", ImVec2(100, 22))) {
                        StorageManager::exportSamplesToCsv(dsp.getSamples(), targetSpeed);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("X", ImVec2(30, 22))) {
                        sessionFinished = false;
                    }

                    ImGui::TextColored(GrooveTheme::TextBright, "Avg: %.2f RPM | Pitch: %+.2f%% | Wow: %.3f%% | Wobble: %.3fg",
                        lastReport.averageRpm, lastReport.pitchDeviationPercent, lastReport.wowPercent, lastReport.wobbleG);
                }
                ImGui::EndChild();
                ImGui::PopStyleColor();
            }
        }
        ImGui::EndChild();

        ImGui::End();

        // Render Modals
        wizard.render(sensorMgr, deltaTime);
        infoDlg.render();

        // 7. ImGui & SDL_Renderer Render Pass
        ImGui::Render();
        SDL_SetRenderDrawColor(renderer, 15, 20, 25, 255);
        SDL_RenderClear(renderer);
        ImGui_ImplSDLRenderer2_RenderDrawData(ImGui::GetDrawData(), renderer);
        SDL_RenderPresent(renderer);
        if (frameNum == 1) {
            logBoot("[GrooveSpeed] Frame 1 rendered successfully!");
        }
    }

    // 8. Cleanup & Shutdown
    ImGui_ImplSDLRenderer2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
