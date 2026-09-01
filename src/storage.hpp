#ifndef STORAGE_HPP
#define STORAGE_HPP

#include "dsp_engine.hpp"
#include <vector>
#include <string>

struct CalibrationConfig {
    float gyroZeroBias = 0.0f;
    float calibrationOffset = 0.0f;
};

class StorageManager {
public:
    static bool ensureDataDirectory();
    static bool ensurePictureDirectory();
    
    static bool saveSession(const SessionRecord& record);
    static bool exportSamplesToCsv(const std::vector<MeasurementPoint>& points, float targetSpeed);
    static bool exportReportCardPng(const SessionRecord& record);
    
    static std::vector<SessionRecord> loadHistory();
    static bool deleteHistoryRecord(size_t index);
    static bool saveHistory(const std::vector<SessionRecord>& history);
    
    static bool saveCalibration(const CalibrationConfig& config);
    static CalibrationConfig loadCalibration();
};

#endif // STORAGE_HPP
