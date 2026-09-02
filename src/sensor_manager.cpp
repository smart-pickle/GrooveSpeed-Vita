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

    // 4. Platter Wobble & Motor Rumble (AC-Coupled High-Pass Filtering & Speed Gating)
    // Tracks running DC gravity vector to decouple static shelf/table tilt
    if (!m_accelInitialized) {
        m_accelDcX = state.acceleration.x;
        m_accelDcY = state.acceleration.y;
        m_accelDcZ = state.acceleration.z;
        m_accelInitialized = true;
    } else {
        // Slow time constant (~1-2s) to track static orientation without absorbing mechanical vibrations
        const float dcAlpha = 0.02f;
        m_accelDcX = dcAlpha * state.acceleration.x + (1.0f - dcAlpha) * m_accelDcX;
        m_accelDcY = dcAlpha * state.acceleration.y + (1.0f - dcAlpha) * m_accelDcY;
        m_accelDcZ = dcAlpha * state.acceleration.z + (1.0f - dcAlpha) * m_accelDcZ;
    }

    // AC vibration component for Motor Rumble (subtracts static gravity vector completely)
    float acX = state.acceleration.x - m_accelDcX;
    float acY = state.acceleration.y - m_accelDcY;
    float acZ = state.acceleration.z - m_accelDcZ;
    float acMagnitude = std::sqrt(acX * acX + acY * acY + acZ * acZ);
    float netRumble = std::max(0.0f, acMagnitude - 0.003f);

    // Dynamic Platter Wobble (cyclic variation from static equilibrium magnitude)
    float currentTotalAccel = std::sqrt(
        state.acceleration.x * state.acceleration.x +
        state.acceleration.y * state.acceleration.y +
        state.acceleration.z * state.acceleration.z
    );
    float staticTotalAccel = std::sqrt(
        m_accelDcX * m_accelDcX +
        m_accelDcY * m_accelDcY +
        m_accelDcZ * m_accelDcZ
    );
    float rawWobble = std::abs(currentTotalAccel - staticTotalAccel);
    float netWobble = std::max(0.0f, rawWobble - 0.002f);

    // Speed Gating: When turntable is motionless or stopped (< 5 RPM), clamp meters to 0.000g
    if (m_data.rpm < 5.0f) {
        m_smoothedWobble = 0.0f;
        m_smoothedRumble = 0.0f;
        m_data.platterWobble = 0.0f;
        m_data.motorRumble = 0.0f;
    } else {
        // Smooth fade-in between 5.0 and 10.0 RPM
        float fade = std::min(1.0f, (m_data.rpm - 5.0f) / 5.0f);
        m_smoothedWobble = 0.1f * netWobble + 0.9f * m_smoothedWobble;
        m_smoothedRumble = 0.1f * netRumble + 0.9f * m_smoothedRumble;
        m_data.platterWobble = m_smoothedWobble * fade;
        m_data.motorRumble = m_smoothedRumble * fade;
    }
}
