#ifndef SENSOR_MANAGER_HPP
#define SENSOR_MANAGER_HPP

#include <psp2/motion.h>
#include <psp2/ctrl.h>
#include <psp2/types.h>
#include <psp2/sysmodule.h>
#include <cmath>

struct SensorData {
    float rpm = 0.0f;
    float instantRpm = 0.0f;
    float rawRpm = 0.0f;
    float currentOrientation = 0.0f; // 0 to 360 degrees
    float platterWobble = 0.0f;      // g-force vertical
    float motorRumble = 0.0f;        // g-force high frequency
    bool isFlat = true;
};

class TurntableSensorManager {
public:
    TurntableSensorManager();
    ~TurntableSensorManager();

    bool init();
    void update(float deltaTime);
    
    SensorData getData() const { return m_data; }

    void calibrateZeroGyro();
    void setGyroZeroBias(float bias) { m_gyroZeroBias = bias; }
    float getGyroZeroBias() const { return m_gyroZeroBias; }

    void setCalibrationOffset(float offsetRpm) { m_calibrationOffset = offsetRpm; }
    float getCalibrationOffset() const { return m_calibrationOffset; }
    void resetCalibration() { m_gyroZeroBias = 0.0f; m_calibrationOffset = 0.0f; }

    // Demo / Emulator mode for Vita3K or testing without physical turntable
    void setDemoMode(bool enable) { m_demoMode = enable; }
    bool isDemoMode() const { return m_demoMode; }
    void setDemoTargetRpm(float targetRpm) { m_demoTargetRpm = targetRpm; }

    // Haptic vibration feedback (small motor, large motor intensity: 0 to 255)
    void triggerHapticVibration(uint8_t smallMotor, uint8_t largeMotor);

private:
    SensorData m_data;
    bool m_isSampling = false;
    bool m_demoMode = false;
    float m_demoTargetRpm = 33.3333f;
    float m_demoTime = 0.0f;
    
    float m_gyroZeroBias = 0.0f;
    float m_calibrationOffset = 0.0f;
    
    // Low-pass filter smoothing coefficient
    const float m_alpha = 0.05f;
    float m_smoothedRpm = 0.0f;

    // Orientation tracking
    float m_accumulatedAngle = 0.0f;

    // Dynamic DC accelerometer tracking for AC vibration / rumble decoupling
    bool m_accelInitialized = false;
    float m_accelDcX = 0.0f;
    float m_accelDcY = 0.0f;
    float m_accelDcZ = -1.0f;
    float m_smoothedWobble = 0.0f;
    float m_smoothedRumble = 0.0f;
};

#endif // SENSOR_MANAGER_HPP
