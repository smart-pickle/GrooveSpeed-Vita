#include "storage.hpp"
#include <psp2/io/stat.h>
#include <fstream>
#include <sstream>
#include <ctime>
#include <cstdlib>

const char* DATA_DIR = "ux0:data/groovespeed";
const char* PICTURE_DIR = "ux0:picture/GrooveSpeed";
const char* HISTORY_FILE = "ux0:data/groovespeed/history.csv";
const char* CONFIG_FILE = "ux0:data/groovespeed/config.txt";

bool StorageManager::ensureDataDirectory() {
    sceIoMkdir("ux0:data", 0777);
    sceIoMkdir(DATA_DIR, 0777);
    return true;
}

bool StorageManager::ensurePictureDirectory() {
    sceIoMkdir("ux0:picture", 0777);
    sceIoMkdir(PICTURE_DIR, 0777);
    return true;
}

bool StorageManager::saveSession(const SessionRecord& record) {
    ensureDataDirectory();

    std::ofstream file(HISTORY_FILE, std::ios::app);
    if (!file.is_open()) return false;

    file << record.timestampMs << ","
         << record.targetSpeed << ","
         << record.averageRpm << ","
         << record.wowPercent << ","
         << record.pitchDeviationPercent << ","
         << record.wobbleG << ","
         << record.rumbleG << ","
         << record.totalRevolutions << ","
         << (record.weightingMode == WowWeighting::DIN ? "DIN" : "UNWEIGHTED") << "\n";

    file.close();
    return true;
}

bool StorageManager::exportSamplesToCsv(const std::vector<MeasurementPoint>& points, float targetSpeed) {
    if (points.empty()) return false;
    ensureDataDirectory();

    std::string filename = std::string(DATA_DIR) + "/GrooveSpeed_Samples.csv";
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << "TimestampMs,TimeOffsetSec,RPM,AngleDeg,WobbleG,RumbleG\n";
    int64_t firstMs = points.front().timestampMs;

    for (const auto& pt : points) {
        float offsetSec = static_cast<float>(pt.timestampMs - firstMs) / 1000.0f;
        file << pt.timestampMs << ","
             << offsetSec << ","
             << pt.rpm << ","
             << pt.angle << ","
             << pt.wobble << ","
             << pt.rumble << "\n";
    }

    file.close();
    return true;
}

bool StorageManager::exportReportCardPng(const SessionRecord& record) {
    ensurePictureDirectory();

    std::string filename = std::string(PICTURE_DIR) + "/GrooveSpeed_Report.txt";
    std::ofstream file(filename);
    if (!file.is_open()) return false;

    file << "========================================\n";
    file << "   GROOVESPEED TURNTABLE DIAGNOSTICS   \n";
    file << "========================================\n\n";
    file << "Target Speed:     " << record.targetSpeed << " RPM\n";
    file << "Measured Avg RPM: " << record.averageRpm << " RPM\n";
    file << "Pitch Deviation:  " << record.pitchDeviationPercent << " %\n";
    file << "Wow & Flutter:    " << record.wowPercent << " % (" 
         << (record.weightingMode == WowWeighting::DIN ? "DIN 45507" : "UNWEIGHTED") << ")\n";
    file << "Platter Wobble:   " << record.wobbleG << " G\n";
    file << "Motor Rumble:     " << record.rumbleG << " G (FFT)\n";
    file << "Total Revolutions:" << record.totalRevolutions << "\n";

    file.close();
    return true;
}

std::vector<SessionRecord> StorageManager::loadHistory() {
    ensureDataDirectory();
    std::vector<SessionRecord> history;

    FILE* checkFile = fopen(HISTORY_FILE, "r");
    if (!checkFile) return history;
    fclose(checkFile);

    std::ifstream file(HISTORY_FILE);
    if (!file.is_open()) return history;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        std::stringstream ss(line);
        std::string token;

        SessionRecord rec{};
        if (std::getline(ss, token, ',')) rec.timestampMs = std::atoll(token.c_str());
        if (std::getline(ss, token, ',')) rec.targetSpeed = static_cast<float>(std::atof(token.c_str()));
        if (std::getline(ss, token, ',')) rec.averageRpm = static_cast<float>(std::atof(token.c_str()));
        if (std::getline(ss, token, ',')) rec.wowPercent = static_cast<float>(std::atof(token.c_str()));
        if (std::getline(ss, token, ',')) rec.pitchDeviationPercent = static_cast<float>(std::atof(token.c_str()));
        if (std::getline(ss, token, ',')) rec.wobbleG = static_cast<float>(std::atof(token.c_str()));
        if (std::getline(ss, token, ',')) rec.rumbleG = static_cast<float>(std::atof(token.c_str()));
        if (std::getline(ss, token, ',')) rec.totalRevolutions = std::atoi(token.c_str());
        if (std::getline(ss, token, ',')) {
            rec.weightingMode = (token == "DIN") ? WowWeighting::DIN : WowWeighting::UNWEIGHTED;
        }
        history.push_back(rec);
    }

    file.close();
    return history;
}

bool StorageManager::saveHistory(const std::vector<SessionRecord>& history) {
    ensureDataDirectory();
    std::ofstream file(HISTORY_FILE);
    if (!file.is_open()) return false;

    for (const auto& record : history) {
        file << record.timestampMs << ","
             << record.targetSpeed << ","
             << record.averageRpm << ","
             << record.wowPercent << ","
             << record.pitchDeviationPercent << ","
             << record.wobbleG << ","
             << record.rumbleG << ","
             << record.totalRevolutions << ","
             << (record.weightingMode == WowWeighting::DIN ? "DIN" : "UNWEIGHTED") << "\n";
    }

    file.close();
    return true;
}

bool StorageManager::deleteHistoryRecord(size_t index) {
    auto history = loadHistory();
    if (index >= history.size()) return false;
    history.erase(history.begin() + index);
    return saveHistory(history);
}

bool StorageManager::saveCalibration(const CalibrationConfig& config) {
    ensureDataDirectory();
    std::ofstream file(CONFIG_FILE);
    if (!file.is_open()) return false;

    file << config.gyroZeroBias << "," << config.calibrationOffset << "\n";
    file.close();
    return true;
}

CalibrationConfig StorageManager::loadCalibration() {
    ensureDataDirectory();
    CalibrationConfig config{};

    FILE* checkFile = fopen(CONFIG_FILE, "r");
    if (!checkFile) return config;
    fclose(checkFile);

    std::ifstream file(CONFIG_FILE);
    if (!file.is_open()) return config;

    std::string line;
    if (std::getline(file, line)) {
        std::stringstream ss(line);
        std::string token;
        if (std::getline(ss, token, ',')) config.gyroZeroBias = static_cast<float>(std::atof(token.c_str()));
        if (std::getline(ss, token, ',')) config.calibrationOffset = static_cast<float>(std::atof(token.c_str()));
    }

    file.close();
    return config;
}
