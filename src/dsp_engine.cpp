#include "dsp_engine.hpp"
#include <numeric>
#include <algorithm>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

DspEngine::DspEngine() {}

void DspEngine::resetSession(float targetSpeed, WowWeighting weighting, int durationSeconds) {
    m_targetSpeed = targetSpeed;
    m_weightingMode = weighting;
    m_durationSeconds = durationSeconds;
    
    if (durationSeconds == -1) {
        m_maxSamples = 30000; // Large buffer for continuous mode
    } else {
        m_maxSamples = static_cast<size_t>(durationSeconds * 100); // 100 samples/sec
    }
    
    m_samples.clear();
    m_samples.reserve(m_maxSamples);
}

void DspEngine::addSample(float rpm, float angle, float wobble, float rumble, int64_t timestampMs) {
    if (m_durationSeconds != -1 && m_samples.size() >= m_maxSamples) return;
    m_samples.push_back({timestampMs, rpm, angle, wobble, rumble});
}

float DspEngine::getProgressPercent() const {
    if (m_durationSeconds == -1) return 100.0f;
    if (m_maxSamples == 0) return 100.0f;
    return (static_cast<float>(m_samples.size()) / static_cast<float>(m_maxSamples)) * 100.0f;
}

// 1. IQR Outlier Rejection Filter
std::vector<float> DspEngine::removeOutliersIQR(const std::vector<float>& input) const {
    if (input.size() < 4) return input;
    std::vector<float> sorted = input;
    std::sort(sorted.begin(), sorted.end());

    float q1 = sorted[static_cast<size_t>(sorted.size() * 0.25f)];
    float q3 = sorted[static_cast<size_t>(sorted.size() * 0.75f)];
    float iqr = q3 - q1;

    float lowerBound = q1 - 1.5f * iqr;
    float upperBound = q3 + 1.5f * iqr;

    std::vector<float> filtered;
    filtered.reserve(input.size());
    for (float val : input) {
        if (val >= lowerBound && val <= upperBound) {
            filtered.push_back(val);
        }
    }
    return filtered;
}

// 2. 2nd-Order IIR DIN 45507 Bandpass Filter (Fs=100Hz, Center=4.0Hz, Q=0.707)
std::vector<float> DspEngine::applyDin2ndOrderIIR(const std::vector<float>& input, float averageRpm) const {
    if (input.size() < 3) return input;

    // Filter Coefficients: b0=0.293, b1=0.0, b2=-0.293, a1=1.143, a2=0.413
    const float b0 = 0.293f, b1 = 0.0f, b2 = -0.293f;
    const float a1 = 1.143f, a2 = 0.413f;

    std::vector<float> filtered;
    filtered.reserve(input.size());

    float x1 = 0.0f, x2 = 0.0f;
    float y1 = 0.0f, y2 = 0.0f;

    for (float rpm : input) {
        float x = rpm - averageRpm;
        float y = b0 * x + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
        filtered.push_back(averageRpm + y);

        x2 = x1; x1 = x;
        y2 = y1; y1 = y;
    }
    return filtered;
}

// 3. RMS Wow & Flutter Calculation
float DspEngine::computeRmsWow(const std::vector<float>& input, float averageRpm) const {
    if (input.empty() || averageRpm <= 0.0f) return 0.0f;

    double sumSquaredDevs = 0.0;
    for (float val : input) {
        double dev = static_cast<double>(val - averageRpm);
        sumSquaredDevs += dev * dev;
    }

    double rms = std::sqrt(sumSquaredDevs / static_cast<double>(input.size()));
    // Convert RMS to peak percentage wow (assuming sine wave fluctuation: peak = RMS * sqrt(2))
    float wowPercent = static_cast<float>(((rms * std::sqrt(2.0)) / static_cast<double>(averageRpm)) * 100.0);
    return wowPercent;
}

// 4. Unweighted Peak-to-Peak Wow Calculation
float DspEngine::computeUnweightedWow(const std::vector<float>& input, float averageRpm) const {
    if (input.empty() || averageRpm <= 0.0f) return 0.0f;
    auto [minIt, maxIt] = std::minmax_element(input.begin(), input.end());
    float peakToPeak = (*maxIt - *minIt) / 2.0f;
    return (peakToPeak / averageRpm) * 100.0f;
}

// 5. Radix-2 Cooley-Tukey FFT for Motor Rumble Spectral Analysis
float DspEngine::computeCooleyTukeyFftRumble(const std::vector<float>& rumbleBuffer) const {
    if (rumbleBuffer.empty()) return 0.0f;

    // Power of 2 buffer length
    size_t n = 1;
    while (n < rumbleBuffer.size()) n *= 2;
    n /= 2;
    if (n < 4) return 0.0f;

    std::vector<double> real(n, 0.0);
    std::vector<double> imag(n, 0.0);
    for (size_t i = 0; i < n; ++i) {
        real[i] = static_cast<double>(rumbleBuffer[i]);
    }

    // Bit reversal permutation
    size_t j = 0;
    for (size_t i = 0; i < n - 1; ++i) {
        if (i < j) {
            std::swap(real[i], real[j]);
            std::swap(imag[i], imag[j]);
        }
        size_t k = n / 2;
        while (k <= j) {
            j -= k;
            k /= 2;
        }
        j += k;
    }

    // Cooley-Tukey FFT
    for (size_t len = 2; len <= n; len <<= 1) {
        double ang = -2.0 * M_PI / static_cast<double>(len);
        double wlen_r = std::cos(ang);
        double wlen_i = std::sin(ang);

        for (size_t i = 0; i < n; i += len) {
            double w_r = 1.0;
            double w_i = 0.0;
            for (size_t k = 0; k < len / 2; ++k) {
                size_t u = i + k;
                size_t v = i + k + len / 2;

                double tr = w_r * real[v] - w_i * imag[v];
                double ti = w_r * imag[v] + w_i * real[v];

                real[v] = real[u] - tr;
                imag[v] = imag[u] - ti;
                real[u] += tr;
                imag[u] += ti;

                double next_w_r = w_r * wlen_r - w_i * wlen_i;
                double next_w_i = w_r * wlen_i + w_i * wlen_r;
                w_r = next_w_r;
                w_i = next_w_i;
            }
        }
    }

    // Sum spectral magnitude in motor rumble frequency band
    double rumbleSum = 0.0;
    size_t startBin = std::max<size_t>(1, n / 10);
    size_t endBin = std::min<size_t>(n / 2, (n * 6) / 10);

    for (size_t i = startBin; i < endBin; ++i) {
        rumbleSum += std::sqrt(real[i] * real[i] + imag[i] * imag[i]);
    }

    return static_cast<float>(rumbleSum / static_cast<double>(n));
}

SessionRecord DspEngine::computeFinalReport() const {
    SessionRecord record{};
    record.targetSpeed = m_targetSpeed;
    record.weightingMode = m_weightingMode;

    if (m_samples.empty()) return record;

    std::vector<float> rawRpms;
    std::vector<float> wobbles;
    std::vector<float> rumbles;
    rawRpms.reserve(m_samples.size());
    wobbles.reserve(m_samples.size());
    rumbles.reserve(m_samples.size());

    for (const auto& pt : m_samples) {
        rawRpms.push_back(pt.rpm);
        wobbles.push_back(pt.wobble);
        rumbles.push_back(pt.rumble);
    }

    // Filter outliers using IQR
    std::vector<float> cleanRpms = removeOutliersIQR(rawRpms);
    if (cleanRpms.empty()) cleanRpms = rawRpms;

    float avgRpm = std::accumulate(cleanRpms.begin(), cleanRpms.end(), 0.0f) / static_cast<float>(cleanRpms.size());
    record.averageRpm = avgRpm;

    // Pitch Deviation %
    record.pitchDeviationPercent = ((avgRpm - m_targetSpeed) / m_targetSpeed) * 100.0f;

    // Wow & Flutter
    if (m_weightingMode == WowWeighting::DIN) {
        std::vector<float> dinFiltered = applyDin2ndOrderIIR(cleanRpms, avgRpm);
        record.wowPercent = computeRmsWow(dinFiltered, avgRpm);
    } else {
        record.wowPercent = computeUnweightedWow(cleanRpms, avgRpm);
    }

    // Wobble & Cooley-Tukey FFT Motor Rumble
    record.wobbleG = std::accumulate(wobbles.begin(), wobbles.end(), 0.0f) / static_cast<float>(wobbles.size());
    record.rumbleG = computeCooleyTukeyFftRumble(rumbles);

    // Revolutions
    float durationSec = (m_durationSeconds > 0) ? static_cast<float>(m_durationSeconds) : 10.0f;
    record.totalRevolutions = static_cast<int>(std::round((avgRpm / 60.0f) * durationSec));

    return record;
}
