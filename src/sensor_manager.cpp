#include "sensor_manager.hpp"
#include <psp2/power.h>
#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

TurntableSensorManager::TurntableSensorManager() {}

TurntableSensorManager::~TurntableSensorManager() {
    if (m_isSampling && !m_demoMode) {
        sceMotionStopSampling();
    }
}

bool TurntableSensorManager::init() {
    // 1. Set controller and touch sampling modes
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);

    // 2. Initialize motion sampling
    int res = sceMotionStartSampling();
    if (res >= 0) {
        m_isSampling = true;
    }
    // Physical hardware sensors active by default
    m_demoMode = false;
    return true;
}

void TurntableSensorManager::triggerHapticVibration(uint8_t smallMotor, uint8_t largeMotor) {
    // Controller actuator rumble only on DualShock port 1
    if (m_demoMode) return;

    SceCtrlActuator actuator{};
    actuator.small = smallMotor;
    actuator.large = largeMotor;
    sceCtrlSetActuator(1, &actuator);
}

void TurntableSensorManager::calibrateZeroGyro() {
    if (!m_isSampling || m_demoMode) return;

    // Take average over 50 samples while stationary
    float sumWz = 0.0f;
    const int numSamples = 50;

    for (int i = 0; i < numSamples; ++i) {
        SceMotionState state;
        if (sceMotionGetState(&state) >= 0) {
            sumWz += state.angularVelocity.z;
        }
    }

    m_gyroZeroBias = sumWz / static_cast<float>(numSamples);
}

void TurntableSensorManager::update(float deltaTime) {
    if (!m_isSampling) return;

    if (m_demoMode) {
        // Synthetic motion generator for Vita3K emulator or demo testing
        m_demoTime += deltaTime;
        
        // Target speed + 4Hz Wow/Flutter + 0.55Hz cyclic wobble + random micro variation
        float wowSignal = 0.045f * std::sin(2.0f * M_PI * 4.0f * m_demoTime);
        float cyclicWobble = 0.025f * std::cos(2.0f * M_PI * (m_demoTargetRpm / 60.0f) * m_demoTime);
        float noise = 0.008f * std::sin(2.0f * M_PI * 19.3f * m_demoTime);

        float rawCalculatedRpm = m_demoTargetRpm + wowSignal + cyclicWobble + noise + m_calibrationOffset;

        m_data.rawRpm = rawCalculatedRpm;
        m_data.instantRpm = rawCalculatedRpm;
        
        // Low-pass filter for smooth main display RPM
        m_smoothedRpm = m_alpha * rawCalculatedRpm + (1.0f - m_alpha) * m_smoothedRpm;
        if (m_smoothedRpm < 0.1f) m_smoothedRpm = rawCalculatedRpm;
        m_data.rpm = m_smoothedRpm;

        // 360 degree rotational angle tracking
        m_accumulatedAngle += (rawCalculatedRpm / 60.0f) * 360.0f * deltaTime;
        m_data.currentOrientation = std::fmod(m_accumulatedAngle, 360.0f);
        if (m_data.currentOrientation < 0.0f) m_data.currentOrientation += 360.0f;

        // Platter wobble & bearing rumble simulation
        m_data.platterWobble = 0.012f + 0.008f * std::sin(2.0f * M_PI * (m_demoTargetRpm / 60.0f) * m_demoTime);
        m_data.motorRumble = 0.004f + 0.003f * std::sin(2.0f * M_PI * 50.0f * m_demoTime);
        m_data.isFlat = true;
        return;
    }

    // Physical Hardware Sensor path
    SceMotionState state{};
    if (sceMotionGetState(&state) < 0) {
        // Switch to demo mode if sensor query fails
        m_demoMode = true;
        return;
    }

    // 1. Check if device is laying reasonably flat (gravity vector on Z)
    float accelMagnitude = std::sqrt(
        state.acceleration.x * state.acceleration.x +
        state.acceleration.y * state.acceleration.y +
        state.acceleration.z * state.acceleration.z
    );

    // 1.0g on Z axis indicates flat placement
    float zRatio = std::abs(state.acceleration.z) / (accelMagnitude > 0.001f ? accelMagnitude : 1.0f);
    m_data.isFlat = (zRatio > 0.85f); // Angle < 30 degrees from horizontal

    // 2. Gyroscope Z-axis angular velocity -> RPM
    float radPerSecZ = std::abs(state.angularVelocity.z - m_gyroZeroBias);
    float rawCalculatedRpm = (radPerSecZ * 60.0f) / (2.0f * M_PI) + m_calibrationOffset;

    m_data.rawRpm = rawCalculatedRpm;
    m_data.instantRpm = rawCalculatedRpm;

    // Low-pass filter for main display RPM
    m_smoothedRpm = m_alpha * rawCalculatedRpm + (1.0f - m_alpha) * m_smoothedRpm;
    m_data.rpm = m_smoothedRpm;

    // 3. Update rotational angle (degrees)
    float angleDelta = (rawCalculatedRpm / 60.0f) * 360.0f * deltaTime;
    m_accumulatedAngle += angleDelta;
    m_data.currentOrientation = std::fmod(m_accumulatedAngle, 360.0f);
    if (m_data.currentOrientation < 0.0f) m_data.currentOrientation += 360.0f;

    // 4. Platter Wobble & Motor Rumble
    // Earth gravity on PS Vita Z-axis is -1.0g when lying face-up flat.
    // Dynamic wobble is the deviation from static 1.0g magnitude.
    float verticalDev = std::abs(std::abs(state.acceleration.z) - 1.0f);
    m_data.platterWobble = (m_data.rpm > 1.0f) ? verticalDev : (verticalDev * 0.1f);
    m_data.motorRumble = std::sqrt(state.acceleration.x * state.acceleration.x + state.acceleration.y * state.acceleration.y);
}
