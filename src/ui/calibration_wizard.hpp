#ifndef CALIBRATION_WIZARD_HPP
#define CALIBRATION_WIZARD_HPP

#include "imgui.h"
#include "../sensor_manager.hpp"
#include <vector>

enum class WizardStep {
    IDLE,
    STEP1_STATIC_ZEROING,
    STEP2_SPIN_REFERENCE,
    SUMMARY
};

enum class StepPhase {
    PREPARE,    // Waiting for user to trigger sampling
    SAMPLING,   // Actively sampling sensor data
    COMPLETED   // Sample acquired, user can proceed to NEXT or RESET
};

class CalibrationWizard {
public:
    CalibrationWizard();

    void open();
    void close();
    bool isOpen() const { return m_step != WizardStep::IDLE; }

    void render(TurntableSensorManager& sensorMgr, float deltaTime);

private:
    WizardStep m_step = WizardStep::IDLE;
    StepPhase m_phase = StepPhase::PREPARE;

    float m_progress = 0.0f;
    float m_timer = 0.0f;

    // Step 1: Static Zeroing state
    float m_step1MeasuredBias = 0.0f;
    bool m_step1Success = false;

    // Step 2: Spin Reference state
    float m_refTargetSpeed = 33.33f;
    float m_step2MeasuredRpm = 0.0f;
    float m_step2CalculatedOffset = 0.0f;
    bool m_step2Success = false;
    std::vector<float> m_samples;

    bool m_shouldOpenPopup = false;
};

#endif // CALIBRATION_WIZARD_HPP
