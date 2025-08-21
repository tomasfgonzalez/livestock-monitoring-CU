/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : LSUManager.h
  * @brief          : LSUManager class, manages connected LSUs in the system
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "LSUManager.h"

#include <algorithm>
#include <cstring>
#include "LSU.h"
#include "general_config.h"
#include "esp_log.h"
#include "display/status.h"

/* Private variables --------------------------------------------------------- */
static const char *LSU_MANAGER_TAG = "LSU Manager";

/* Static constants --------------------------------------------------------- */
const char* LSUManager::NVS_NAMESPACE = "lsu_mgr";
const char* LSUManager::NVS_LSU_COUNT_KEY = "lsu_count";
const char* LSUManager::NVS_LSU_DATA_KEY = "lsu_data";

/* Helper functions ----------------------------------------------------------*/
uint32_t LSUManager::generateLSUId() {
    static uint32_t nextId = 0x02; // Start at 0x02 to avoid conflict with CU
    ESP_LOGI(LSU_MANAGER_TAG, "Generating LSU ID: %lu", nextId);
    return nextId++;
}

uint32_t LSUManager::generateTimeSlot() {
    // If no LSUs exist, start with slot 0
    if (connectedLSUs.empty()) {
        return 0;
    }

    std::vector<uint32_t> existingSlots;
    for (const auto& lsu : connectedLSUs) {
        existingSlots.push_back(lsu.second->getTimeSlotInPeriod());
    }
    std::sort(existingSlots.begin(), existingSlots.end());
    
    // Find the largest gap in the circular period
    uint32_t maxGap = 0;
    uint32_t bestSlot = -1;
    uint32_t n = existingSlots.size();
    for (uint32_t index = 0; index < n; index++) {
        uint32_t current = existingSlots[index];
        uint32_t next = existingSlots[(index + 1) % n];
        
        // Handle circular wrap-around
        uint32_t gap = (next - current);
        if (index == n - 1) {
            gap += TIME_PERIOD_MS;
        }
        
        if (gap > maxGap) {
            maxGap = gap;
            bestSlot = (current + gap / 2) % TIME_PERIOD_MS;
        }
    }
    ESP_LOGI(LSU_MANAGER_TAG, "Generating time slot: %lu", bestSlot);
    return bestSlot;
}

/* Constructor and Destructor ------------------------------------------------*/
LSUManager::LSUManager() {
    ESP_LOGI(LSU_MANAGER_TAG, "Initializing LSU Manager");
    if (!loadFromNVS()) {
        ESP_LOGW(LSU_MANAGER_TAG, "Failed to load LSU data from NVS, starting with empty state");
    }
}

LSUManager::~LSUManager() {
    ESP_LOGI(LSU_MANAGER_TAG, "Destructing LSU Manager, saving data");
    saveToNVS();
    
    // Clean up allocated LSU objects
    for (auto& pair : connectedLSUs) {
        delete pair.second;
    }
    connectedLSUs.clear();
}

/* Function implementations -------------------------------------------------*/
std::pair<LSU*, bool> LSUManager::createLSU() {
    if (connectedLSUs.size() >= MAX_LSU_COUNT) {
        ESP_LOGE(LSU_MANAGER_TAG, "Failed to create LSU. Max LSU count reached.");
        return std::make_pair(nullptr, false);
    }

    uint32_t lsuId = generateLSUId();
    uint32_t timeSlotInPeriod = generateTimeSlot();
    LSU* newLSU = new LSU(lsuId, timeSlotInPeriod);
    
    auto result = connectedLSUs.insert(std::make_pair(lsuId, newLSU));
    update_lsu_count(connectedLSUs.size());
    
    if (result.second) {
        // Add timeout event for the new LSU
        TimeoutEvent event = {lsuId, time(nullptr) + DEFAULT_TIMEOUT_MS};
        timeoutQueue.push(event);
        
        // Save to NVS after successful creation
        saveToNVS();
        
        return std::make_pair(newLSU, true);
    }
    
    return std::make_pair(nullptr, false); // Insert failed
}

bool LSUManager::removeLSU(uint32_t lsuId) {
    auto it = connectedLSUs.find(lsuId);
    if (it != connectedLSUs.end()) {
        connectedLSUs.erase(it);
        update_lsu_count(connectedLSUs.size());
        
        // Save to NVS after removal
        saveToNVS();
        
        return true;
    }
    return false; // LSU not found
}

LSU* LSUManager::getLSU(uint32_t lsuId) {
    auto it = connectedLSUs.find(lsuId);
    if (it != connectedLSUs.end()) {
        return it -> second;
    }
    return nullptr; // LSU not found
}

bool LSUManager::keepaliveLSU(uint32_t lsuId) {
    LSU* lsu = getLSU(lsuId);

    if (lsu != nullptr) {
        time_t currentTime = time(nullptr);
        lsu->setLastConnectionTime(currentTime);

        TimeoutEvent event = {lsuId, currentTime + DEFAULT_TIMEOUT_MS};
        timeoutQueue.push(event);
        
        // Save to NVS after keepalive update
        saveToNVS();
        
        return true;
    }
    return false; // LSU not found
}

void LSUManager::processTimeouts() {
    time_t currentTime = time(nullptr);
    bool timeoutOccurred = false;

    while (!timeoutQueue.empty() && timeoutQueue.top().timeoutTime <= currentTime) {
        TimeoutEvent event = timeoutQueue.top();
        timeoutQueue.pop();
        
        LSU* lsu = getLSU(event.lsuId);
        if (lsu != nullptr) {
            if (lsu->getLastConnectionTime() + DEFAULT_TIMEOUT_MS <= currentTime) {
                // LSU has timed out, remove it
                connectedLSUs.erase(event.lsuId);
                timeoutOccurred = true;
            }
        }
    }
    
    // Save to NVS if any timeouts occurred
    if (timeoutOccurred) {
        saveToNVS();
    }
}

/* NVS Storage Methods ------------------------------------------------------*/
size_t LSUManager::serializeLSUData(uint8_t* buffer, size_t bufferSize) const {
    if (!buffer || bufferSize == 0) {
        return 0;
    }
    
    size_t offset = 0;
    size_t lsuCount = connectedLSUs.size();
    
    // Write LSU count (4 bytes)
    if (offset + sizeof(uint32_t) <= bufferSize) {
        memcpy(buffer + offset, &lsuCount, sizeof(uint32_t));
        offset += sizeof(uint32_t);
    } else {
        return 0;
    }
    
    // Write each LSU data (id: 4 bytes, timeSlot: 4 bytes, lastConnectionTime: 8 bytes)
    for (const auto& pair : connectedLSUs) {
        const LSU* lsu = pair.second;
        if (offset + 16 <= bufferSize) { // 4 + 4 + 8 = 16 bytes per LSU
            uint32_t id = lsu->getId();
            uint32_t timeSlot = lsu->getTimeSlotInPeriod();
            time_t lastConnection = lsu->getLastConnectionTime();
            
            memcpy(buffer + offset, &id, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(buffer + offset, &timeSlot, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(buffer + offset, &lastConnection, sizeof(time_t));
            offset += sizeof(time_t);
        } else {
            return 0; // Buffer too small
        }
    }
    
    return offset;
}

bool LSUManager::deserializeLSUData(const uint8_t* buffer, size_t dataSize) {
    if (!buffer || dataSize < sizeof(uint32_t)) {
        return false;
    }
    
    size_t offset = 0;
    uint32_t lsuCount;
    
    // Read LSU count
    memcpy(&lsuCount, buffer + offset, sizeof(uint32_t));
    offset += sizeof(uint32_t);
    
    // Validate count
    if (lsuCount > MAX_LSU_COUNT || offset + (lsuCount * 16) > dataSize) {
        ESP_LOGE(LSU_MANAGER_TAG, "Invalid LSU count or data size");
        return false;
    }
    
    // Clear existing LSUs
    for (auto& pair : connectedLSUs) {
        delete pair.second;
    }
    connectedLSUs.clear();
    
    // Clear timeout queue
    while (!timeoutQueue.empty()) {
        timeoutQueue.pop();
    }
    
    // Recreate LSUs from stored data
    for (uint32_t i = 0; i < lsuCount; i++) {
        if (offset + 16 <= dataSize) {
            uint32_t id, timeSlot;
            time_t lastConnection;
            
            memcpy(&id, buffer + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(&timeSlot, buffer + offset, sizeof(uint32_t));
            offset += sizeof(uint32_t);
            memcpy(&lastConnection, buffer + offset, sizeof(time_t));
            offset += sizeof(time_t);
            
            // Create new LSU
            LSU* lsu = new LSU(id, timeSlot);
            lsu->setLastConnectionTime(lastConnection);
            
            // Add to map
            connectedLSUs[id] = lsu;
            
            // Add to timeout queue
            TimeoutEvent event = {id, lastConnection + DEFAULT_TIMEOUT_MS};
            timeoutQueue.push(event);
        } else {
            ESP_LOGE(LSU_MANAGER_TAG, "Incomplete LSU data");
            return false;
        }
    }
    
    ESP_LOGI(LSU_MANAGER_TAG, "Loaded %lu LSUs from NVS", lsuCount);
    return true;
}

bool LSUManager::saveToNVS() {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_MANAGER_TAG, "Failed to open NVS namespace: %s", esp_err_to_name(err));
        return false;
    }
    
    // Calculate required buffer size
    size_t bufferSize = sizeof(uint32_t) + (connectedLSUs.size() * 16); // count + (id + timeSlot + lastConnection) per LSU
    uint8_t* buffer = new uint8_t[bufferSize];
    
    if (!buffer) {
        ESP_LOGE(LSU_MANAGER_TAG, "Failed to allocate buffer for serialization");
        nvs_close(nvs_handle);
        return false;
    }
    
    // Serialize data
    size_t dataSize = serializeLSUData(buffer, bufferSize);
    if (dataSize == 0) {
        ESP_LOGE(LSU_MANAGER_TAG, "Failed to serialize LSU data");
        delete[] buffer;
        nvs_close(nvs_handle);
        return false;
    }
    
    // Save to NVS
    err = nvs_set_blob(nvs_handle, NVS_LSU_DATA_KEY, buffer, dataSize);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_MANAGER_TAG, "Failed to save LSU data to NVS: %s", esp_err_to_name(err));
        delete[] buffer;
        nvs_close(nvs_handle);
        return false;
    }
    
    // Commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_MANAGER_TAG, "Failed to commit NVS changes: %s", esp_err_to_name(err));
        delete[] buffer;
        nvs_close(nvs_handle);
        return false;
    }
    
    ESP_LOGI(LSU_MANAGER_TAG, "Successfully saved %lu LSUs to NVS", connectedLSUs.size());
    
    delete[] buffer;
    nvs_close(nvs_handle);
    return true;
}

bool LSUManager::loadFromNVS() {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS namespace
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_MANAGER_TAG, "Failed to open NVS namespace: %s", esp_err_to_name(err));
        return false;
    }
    
    // Get data size
    size_t dataSize = 0;
    err = nvs_get_blob(nvs_handle, NVS_LSU_DATA_KEY, nullptr, &dataSize);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGI(LSU_MANAGER_TAG, "No LSU data found in NVS, starting fresh");
            nvs_close(nvs_handle);
            return true; // Not an error, just no data yet
        }
        ESP_LOGE(LSU_MANAGER_TAG, "Failed to get LSU data size from NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }
    
    // Allocate buffer
    uint8_t* buffer = new uint8_t[dataSize];
    if (!buffer) {
        ESP_LOGE(LSU_MANAGER_TAG, "Failed to allocate buffer for deserialization");
        nvs_close(nvs_handle);
        return false;
    }
    
    // Load data from NVS
    err = nvs_get_blob(nvs_handle, NVS_LSU_DATA_KEY, buffer, &dataSize);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_MANAGER_TAG, "Failed to load LSU data from NVS: %s", esp_err_to_name(err));
        delete[] buffer;
        nvs_close(nvs_handle);
        return false;
    }
    
    // Deserialize data
    bool success = deserializeLSUData(buffer, dataSize);
    
    delete[] buffer;
    nvs_close(nvs_handle);
    
    return success;
}
