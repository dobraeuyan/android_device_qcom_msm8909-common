#define LOG_TAG "android.hardware.thermal@2.0-service-msm8909"

#include <algorithm>
#include <array>
#include <cctype>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dirent.h>
#include <fstream>
#include <map>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <android-base/logging.h>
#include <hidl/HidlTransportSupport.h>

#include "Thermal.h"

namespace android {
namespace hardware {
namespace thermal {
namespace V2_0 {
namespace implementation {

using ::android::hardware::interfacesEqual;
using ::android::hardware::thermal::V1_0::ThermalStatus;
using ::android::hardware::thermal::V1_0::ThermalStatusCode;
using V1_0TemperatureType = ::android::hardware::thermal::V1_0::TemperatureType;

namespace {

constexpr char kThermalRoot[] = "/sys/devices/virtual/thermal";
constexpr size_t kSeverityCount = 7;
constexpr int kMaxTrips = 10;

struct Zone {
    std::string name;
    TemperatureType type;
    float temp;
    std::array<float, kSeverityCount> hot;
    std::array<float, kSeverityCount> cold;
    float vr;
    ThrottlingSeverity severity;
};

std::string trim(const std::string& value) {
    const size_t start = value.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const size_t end = value.find_last_not_of(" \t\r\n");
    return value.substr(start, end - start + 1);
}

bool readFile(const std::string& path, std::string* value) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return false;
    }
    std::getline(file, *value);
    *value = trim(*value);
    return true;
}

bool readInt(const std::string& path, int* value) {
    std::string text;
    if (!readFile(path, &text)) {
        return false;
    }
    char* end = nullptr;
    const long parsed = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str()) {
        return false;
    }
    *value = static_cast<int>(parsed);
    return true;
}

bool readUint64(const std::string& path, uint64_t* value) {
    std::string text;
    if (!readFile(path, &text)) {
        return false;
    }
    char* end = nullptr;
    const unsigned long long parsed = std::strtoull(text.c_str(), &end, 10);
    if (end == text.c_str()) {
        return false;
    }
    *value = static_cast<uint64_t>(parsed);
    return true;
}

std::vector<std::string> listEntries(const std::string& root, const std::string& prefix) {
    std::vector<std::string> entries;
    DIR* dir = opendir(root.c_str());
    if (dir == nullptr) {
        return entries;
    }
    struct dirent* entry = nullptr;
    while ((entry = readdir(dir)) != nullptr) {
        if (std::strncmp(entry->d_name, prefix.c_str(), prefix.size()) == 0) {
            entries.emplace_back(entry->d_name);
        }
    }
    closedir(dir);
    std::sort(entries.begin(), entries.end());
    return entries;
}

std::string lower(const std::string& value) {
    std::string result = value;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

TemperatureType mapType(const std::string& name) {
    const std::string type = lower(name);
    if (type.find("gpu") != std::string::npos) {
        return TemperatureType::GPU;
    }
    if (type.find("cpu") != std::string::npos) {
        return TemperatureType::CPU;
    }
    if (type.find("batt") != std::string::npos || type.find("bms") != std::string::npos) {
        return TemperatureType::BATTERY;
    }
    if (type.find("pa-therm") != std::string::npos || type.find("pa_therm") != std::string::npos) {
        return TemperatureType::POWER_AMPLIFIER;
    }
    if (type.find("skin") != std::string::npos || type.find("xo-therm") != std::string::npos ||
        type.find("pm8916") != std::string::npos) {
        return TemperatureType::SKIN;
    }
    return TemperatureType::UNKNOWN;
}

CoolingType mapCoolingType(const std::string& name) {
    const std::string type = lower(name);
    if (type.find("fan") != std::string::npos) {
        return CoolingType::FAN;
    }
    if (type.find("batt") != std::string::npos) {
        return CoolingType::BATTERY;
    }
    if (type.find("cpu") != std::string::npos || type.find("processor") != std::string::npos) {
        return CoolingType::CPU;
    }
    if (type.find("gpu") != std::string::npos) {
        return CoolingType::GPU;
    }
    if (type.find("modem") != std::string::npos) {
        return CoolingType::MODEM;
    }
    if (type.find("npu") != std::string::npos) {
        return CoolingType::NPU;
    }
    return CoolingType::COMPONENT;
}

ThrottlingSeverity severityFor(const Zone& zone) {
    for (int severity = static_cast<int>(ThrottlingSeverity::SHUTDOWN);
         severity >= static_cast<int>(ThrottlingSeverity::LIGHT); --severity) {
        if (!std::isnan(zone.hot[severity]) && zone.temp >= zone.hot[severity]) {
            return static_cast<ThrottlingSeverity>(severity);
        }
    }
    return ThrottlingSeverity::NONE;
}

std::vector<Zone> readZones() {
    std::vector<Zone> zones;
    for (const auto& entry : listEntries(kThermalRoot, "thermal_zone")) {
        const std::string base = std::string(kThermalRoot) + "/" + entry;
        std::string name;
        int temp = 0;
        if (!readFile(base + "/type", &name) || !readInt(base + "/temp", &temp)) {
            continue;
        }

        Zone zone;
        zone.name = name;
        zone.type = mapType(name);
        zone.temp = static_cast<float>(temp) / 1000.0f;
        zone.hot.fill(NAN);
        zone.cold.fill(NAN);
        zone.vr = NAN;

        std::vector<std::pair<int, std::string>> trips;
        for (int i = 0; i < kMaxTrips; ++i) {
            int tripTemp = 0;
            std::string tripType;
            if (!readInt(base + "/trip_point_" + std::to_string(i) + "_temp", &tripTemp)) {
                break;
            }
            if (!readFile(base + "/trip_point_" + std::to_string(i) + "_type", &tripType)) {
                continue;
            }
            trips.emplace_back(tripTemp, tripType);
        }

        std::sort(trips.begin(), trips.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });

        int level = static_cast<int>(ThrottlingSeverity::LIGHT);
        for (const auto& trip : trips) {
            int severity;
            if (trip.second.find("critical") != std::string::npos) {
                severity = static_cast<int>(ThrottlingSeverity::SHUTDOWN);
            } else {
                severity = level++;
                if (severity >= static_cast<int>(ThrottlingSeverity::SHUTDOWN)) {
                    severity = static_cast<int>(ThrottlingSeverity::CRITICAL);
                }
            }
            zone.hot[severity] = static_cast<float>(trip.first) / 1000.0f;
        }

        zone.severity = severityFor(zone);
        zones.push_back(std::move(zone));
    }
    return zones;
}

std::vector<CoolingDevice_2_0> readCoolingDevices() {
    std::vector<CoolingDevice_2_0> devices;
    for (const auto& entry : listEntries(kThermalRoot, "cooling_device")) {
        const std::string base = std::string(kThermalRoot) + "/" + entry;
        std::string type;
        uint64_t state = 0;
        if (!readFile(base + "/type", &type) || !readUint64(base + "/cur_state", &state)) {
            continue;
        }

        CoolingDevice_2_0 device;
        device.type = mapCoolingType(type);
        device.name = type;
        device.value = state;
        devices.push_back(std::move(device));
    }
    return devices;
}

Temperature_2_0 toTemperature(const Zone& zone) {
    Temperature_2_0 temperature;
    temperature.type = zone.type;
    temperature.name = zone.name;
    temperature.value = zone.temp;
    temperature.throttlingStatus = zone.severity;
    return temperature;
}

TemperatureThreshold toThreshold(const Zone& zone) {
    TemperatureThreshold threshold;
    threshold.type = zone.type;
    threshold.name = zone.name;
    for (size_t i = 0; i < kSeverityCount; ++i) {
        threshold.hotThrottlingThresholds[i] = zone.hot[i];
        threshold.coldThrottlingThresholds[i] = zone.cold[i];
    }
    threshold.vrThrottlingThreshold = zone.vr;
    return threshold;
}

float lowestHot(const Zone& zone) {
    for (size_t i = static_cast<size_t>(ThrottlingSeverity::LIGHT); i < kSeverityCount; ++i) {
        if (!std::isnan(zone.hot[i])) {
            return zone.hot[i];
        }
    }
    return NAN;
}

float highestHot(const Zone& zone) {
    for (int i = static_cast<int>(kSeverityCount) - 1;
         i >= static_cast<int>(ThrottlingSeverity::LIGHT); --i) {
        if (!std::isnan(zone.hot[i])) {
            return zone.hot[i];
        }
    }
    return NAN;
}

V1_0TemperatureType toV1Type(TemperatureType type) {
    switch (type) {
        case TemperatureType::CPU:
            return V1_0TemperatureType::CPU;
        case TemperatureType::GPU:
            return V1_0TemperatureType::GPU;
        case TemperatureType::BATTERY:
            return V1_0TemperatureType::BATTERY;
        case TemperatureType::SKIN:
            return V1_0TemperatureType::SKIN;
        default:
            return V1_0TemperatureType::UNKNOWN;
    }
}

Temperature_1_0 toV1Temperature(const Zone& zone) {
    Temperature_1_0 temperature;
    temperature.type = toV1Type(zone.type);
    temperature.name = zone.name;
    temperature.currentValue = zone.temp;
    temperature.throttlingThreshold = lowestHot(zone);
    const float shutdown =
            zone.hot[static_cast<size_t>(ThrottlingSeverity::SHUTDOWN)];
    temperature.shutdownThreshold = std::isnan(shutdown) ? highestHot(zone) : shutdown;
    temperature.vrThrottlingThreshold = NAN;
    return temperature;
}

}  // namespace

Thermal::Thermal() {
    std::thread([this] { pollCallbacks(); }).detach();
}

Return<void> Thermal::getTemperatures(getTemperatures_cb _hidl_cb) {
    ThermalStatus status;
    status.code = ThermalStatusCode::SUCCESS;

    std::vector<Temperature_1_0> temperatures;
    for (const auto& zone : readZones()) {
        temperatures.push_back(toV1Temperature(zone));
    }

    _hidl_cb(status, temperatures);
    return Void();
}

Return<void> Thermal::getCpuUsages(getCpuUsages_cb _hidl_cb) {
    ThermalStatus status;
    status.code = ThermalStatusCode::SUCCESS;
    std::vector<CpuUsage> cpu_usages;
    _hidl_cb(status, cpu_usages);
    return Void();
}

Return<void> Thermal::getCoolingDevices(getCoolingDevices_cb _hidl_cb) {
    ThermalStatus status;
    status.code = ThermalStatusCode::SUCCESS;
    std::vector<CoolingDevice_1_0> cooling_devices;
    _hidl_cb(status, cooling_devices);
    return Void();
}

Return<void> Thermal::getCurrentTemperatures(bool filterType, TemperatureType type,
                                             getCurrentTemperatures_cb _hidl_cb) {
    ThermalStatus status;
    status.code = ThermalStatusCode::SUCCESS;

    std::vector<Temperature_2_0> temperatures;
    for (const auto& zone : readZones()) {
        if (filterType && zone.type != type) {
            continue;
        }
        temperatures.push_back(toTemperature(zone));
    }

    _hidl_cb(status, temperatures);
    return Void();
}

Return<void> Thermal::getTemperatureThresholds(bool filterType, TemperatureType type,
                                               getTemperatureThresholds_cb _hidl_cb) {
    ThermalStatus status;
    status.code = ThermalStatusCode::SUCCESS;

    std::vector<TemperatureThreshold> thresholds;
    for (const auto& zone : readZones()) {
        if (filterType && zone.type != type) {
            continue;
        }
        thresholds.push_back(toThreshold(zone));
    }

    _hidl_cb(status, thresholds);
    return Void();
}

Return<void> Thermal::getCurrentCoolingDevices(bool filterType, CoolingType type,
                                               getCurrentCoolingDevices_cb _hidl_cb) {
    ThermalStatus status;
    status.code = ThermalStatusCode::SUCCESS;

    std::vector<CoolingDevice_2_0> devices;
    for (const auto& device : readCoolingDevices()) {
        if (filterType && device.type != type) {
            continue;
        }
        devices.push_back(device);
    }

    _hidl_cb(status, devices);
    return Void();
}

Return<void> Thermal::registerThermalChangedCallback(
        const sp<IThermalChangedCallback>& callback, bool filterType, TemperatureType type,
        registerThermalChangedCallback_cb _hidl_cb) {
    ThermalStatus status;
    if (callback == nullptr) {
        status.code = ThermalStatusCode::FAILURE;
        status.debugMessage = "Invalid nullptr callback";
        LOG(ERROR) << status.debugMessage;
        _hidl_cb(status);
        return Void();
    }

    status.code = ThermalStatusCode::SUCCESS;
    std::lock_guard<std::mutex> lock(thermal_callback_mutex_);
    if (std::any_of(callbacks_.begin(), callbacks_.end(), [&](const CallbackSetting& setting) {
            return interfacesEqual(setting.callback, callback);
        })) {
        status.code = ThermalStatusCode::FAILURE;
        status.debugMessage = "Callback already registered";
        LOG(ERROR) << status.debugMessage;
    } else {
        callbacks_.emplace_back(callback, filterType, type);
        LOG(INFO) << "Thermal callback registered, filterType: " << filterType
                  << " type: " << ::android::hardware::thermal::V2_0::toString(type);
    }

    _hidl_cb(status);
    return Void();
}

Return<void> Thermal::unregisterThermalChangedCallback(
        const sp<IThermalChangedCallback>& callback,
        unregisterThermalChangedCallback_cb _hidl_cb) {
    ThermalStatus status;
    if (callback == nullptr) {
        status.code = ThermalStatusCode::FAILURE;
        status.debugMessage = "Invalid nullptr callback";
        LOG(ERROR) << status.debugMessage;
        _hidl_cb(status);
        return Void();
    }

    status.code = ThermalStatusCode::SUCCESS;
    bool removed = false;
    {
        std::lock_guard<std::mutex> lock(thermal_callback_mutex_);
        callbacks_.erase(
                std::remove_if(callbacks_.begin(), callbacks_.end(),
                               [&](const CallbackSetting& setting) {
                                   if (interfacesEqual(setting.callback, callback)) {
                                       removed = true;
                                       return true;
                                   }
                                   return false;
                               }),
                callbacks_.end());
    }

    if (!removed) {
        status.code = ThermalStatusCode::FAILURE;
        status.debugMessage = "Callback was not registered";
        LOG(ERROR) << status.debugMessage;
    }

    _hidl_cb(status);
    return Void();
}

void Thermal::pollCallbacks() {
    std::map<std::string, ThrottlingSeverity> last;
    for (const auto& zone : readZones()) {
        last[zone.name] = zone.severity;
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));

        for (const auto& zone : readZones()) {
            auto it = last.find(zone.name);
            if (it == last.end()) {
                last[zone.name] = zone.severity;
                continue;
            }
            if (it->second == zone.severity) {
                continue;
            }
            it->second = zone.severity;

            const Temperature_2_0 temperature = toTemperature(zone);
            std::vector<CallbackSetting> callbacks;
            {
                std::lock_guard<std::mutex> lock(thermal_callback_mutex_);
                callbacks = callbacks_;
            }
            for (const auto& setting : callbacks) {
                if (setting.is_filter_type && setting.type != temperature.type) {
                    continue;
                }
                setting.callback->notifyThrottling(temperature);
            }
        }
    }
}

}  // namespace implementation
}  // namespace V2_0
}  // namespace thermal
}  // namespace hardware
}  // namespace android
