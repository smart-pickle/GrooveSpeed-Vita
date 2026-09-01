#ifndef DSP_ENGINE_HPP
#define DSP_ENGINE_HPP

#include <vector>
#include <cstdint>
#include <cmath>

enum class WowWeighting {
    DIN,
    UNWEIGHTED
};

struct MeasurementPoint {
    int64_t timestampMs;
    float rpm;
    float angle;
    float wobble;
    float rumble;
};

struct SessionRecord {
    int64_t timestampMs;
    float targetSpeed;
    float averageRpm;
    float wowPercent;
    float pitchDeviationPercent;
    float wobbleG;
    float rumbleG;
    int totalRevolutions;
    WowWeighting weightingMode;
};

class DspEngine {
public:
    DspEngine();

    void resetSession(float targetSpeed, WowWeighting weighting, int durationSeconds = 10);
    void addSample(float rpm, float angle, float wobble, float rumble, int64_t timestampMs);

    bool isSessionComplete() const { 
        if (m_durationSeconds == -1) return false; // Continuous mode
        return m_samples.size() >= m_maxSamples; 
    }
    
    float getProgressPercent() const;

    SessionRecord computeFinalReport() const;

    const std::vector<MeasurementPoint>& getSamples() const { return m_samples; }
    float getTargetSpeed() const { return m_targetSpeed; }

private:
    float m_targetSpeed = 33.33f;
    WowWeighting m_weightingMode = WowWeighting::DIN;
    int m_durationSeconds = 10;
    std::vector<MeasurementPoint> m_samples;
    size_t m_maxSamples = 1000; // ~10 seconds at 100Hz

    // Advanced DSP Algorithms
    std::vector<float> removeOutliersIQR(const std::vector<float>& input) const;
    std::vector<float> applyDin2ndOrderIIR(const std::vector<float>& input, float averageRpm) const;
    float computeRmsWow(const std::vector<float>& input, float averageRpm) const;
    float computeUnweightedWow(const std::vector<float>& input, float averageRpm) const;
    float computeCooleyTukeyFftRumble(const std::vector<float>& rumbleBuffer) const;
};

#endif // DSP_ENGINE_HPP
