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
#include <string>
#include "esp_timer.h"

#include "LSU.h"
#include "general_config.h"
#include "esp_log.h"
#include "display/status.h"

/* Private variables --------------------------------------------------------- */
static const char *LSU_MANAGER_TAG = "LSU Manager";

/* Helper functions ----------------------------------------------------------*/
uint32_t LSUManager::generateLSUId() {
    ESP_LOGI(LSU_MANAGER_TAG, "Generating LSU ID: %lu", nextLSUId);
    return nextLSUId++;
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

void LSUManager::updateNextIdCounter() {
    uint32_t maxId = 0x01; // Start with CU address = 0x01
    for (const auto& pair : connectedLSUs) {
        uint32_t lsuId = pair.first;
        if (lsuId > maxId) {
            maxId = lsuId;
        }
    }
    // Update the next ID counter to be one more than the highest ID
    nextLSUId = maxId + 1;
    ESP_LOGI(LSU_MANAGER_TAG, "Updated next ID counter to start from %lu", nextLSUId);
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
        // Add timeout event for the new LSU (two whole periods)
        int64_t currentTime_us = esp_timer_get_time();
        int64_t timeoutTime_us = currentTime_us + LSU_TIMEOUT_US + LSU_TIMEOUT_PADDING_US;
        TimeoutEvent event = {lsuId, timeoutTime_us};
        timeoutQueue.push(event);
        return std::make_pair(newLSU, true);
    }
    
    return std::make_pair(nullptr, false); // Insert failed
}

bool LSUManager::removeLSU(uint32_t lsuId) {
    auto it = connectedLSUs.find(lsuId);
    if (it != connectedLSUs.end()) {
        // Publish device removal notification to MQTT before removing
        uint32_t lsu_id = it->second->getId();
        
        // Publish device removal notification to MQTT
        std::string topic = "livestock/" + std::to_string(lsu_id) + "/alert";
        std::string payload = "Device manually removed - ID: " + std::to_string(lsu_id);
        extern void mqtt_api_publish(const char *topic, const char *payload);
        mqtt_api_publish(topic.c_str(), payload.c_str());
        
        ESP_LOGI(LSU_MANAGER_TAG, "Published device removal notification to MQTT for LSU %lu", lsu_id);
        
        connectedLSUs.erase(it);
        update_lsu_count(connectedLSUs.size());
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
        int64_t currentTime_us = esp_timer_get_time();
        lsu->setLastConnectionTime(currentTime_us);

        int64_t timeoutTime_us = currentTime_us + LSU_TIMEOUT_US + LSU_TIMEOUT_PADDING_US;
        TimeoutEvent event = {lsuId, timeoutTime_us};
        timeoutQueue.push(event);
        return true;
    }
    return false; // LSU not found
}

void LSUManager::processTimeouts() {
    int64_t currentTime_us = esp_timer_get_time();

    while (!timeoutQueue.empty() && timeoutQueue.top().timeoutTime <= currentTime_us) {
        TimeoutEvent event = timeoutQueue.top();
        timeoutQueue.pop();
        
        LSU* lsu = getLSU(event.lsuId);
        if (lsu != nullptr) {
            // Check if LSU has been silent for longer than timeout period
            int64_t lastConnectionTime_us = lsu->getLastConnectionTime();
            int64_t timeSinceConnection_us = currentTime_us - lastConnectionTime_us;
            
            if (timeSinceConnection_us >= LSU_TIMEOUT_US) {
                // LSU has timed out, send alert and remove it
                uint32_t lsu_id = lsu->getId();
                
                // Send timeout alert to MQTT
                std::string topic = "livestock/" + std::to_string(lsu_id) + "/alert";
                std::string payload = "ALERT: Device timeout - LSU " + std::to_string(lsu_id) + 
                                      " has not communicated for " + std::to_string(LSU_TIMEOUT_US/1000000) + " seconds";
                extern void mqtt_api_publish(const char *topic, const char *payload);
                mqtt_api_publish(topic.c_str(), payload.c_str());
                
                ESP_LOGW(LSU_MANAGER_TAG, "LSU %lu timed out after %lu seconds - removed and alert sent", 
                         lsu_id, (uint32_t) LSU_TIMEOUT_US/1000000);
                
                // Remove the LSU
                connectedLSUs.erase(event.lsuId);
                update_lsu_count(connectedLSUs.size());
            }
        }
    }
}

/* Save/Load functions --------------------------------------------------------- */
std::vector<LSUData> LSUManager::getLsuSerializedData() const {
    std::vector<LSUData> lsuDataVector;
    
    for (const auto& pair : connectedLSUs) {
        LSU* lsu = pair.second;
        LSUData serialize = {
            .id = lsu->getId(),
            .timeSlotInPeriod = lsu->getTimeSlotInPeriod(),
            .lastConnectionTime_us = lsu->getLastConnectionTime()
        };
        lsuDataVector.push_back(serialize);
    }
    
    return lsuDataVector;
}

bool LSUManager::restoreLsuFromSerializedData(const std::vector<LSUData>& lsuDataVector, int64_t savedTimestamp_us) {
    // Clear existing LSUs first
    connectedLSUs.clear();
    while (!timeoutQueue.empty()) {
        timeoutQueue.pop();
    }
    
    // Calculate time offset in microseconds if we have a saved timestamp
    int64_t currentTime_us = esp_timer_get_time();
    int64_t timeOffset_us = currentTime_us - savedTimestamp_us;
    ESP_LOGI(LSU_MANAGER_TAG, "Time offset calculated: %lld microseconds since save", timeOffset_us);
    
    // Restore LSUs from loaded data
    for (const auto& data : lsuDataVector) {
        LSU* lsu = new LSU(data.id, data.timeSlotInPeriod);
        
        // Adjust the last connection time by the time offset (can be negative)
        int64_t adjustedLastConnection_us = data.lastConnectionTime_us + timeOffset_us;
        lsu->setLastConnectionTime(adjustedLastConnection_us);
        
        connectedLSUs.insert(std::make_pair(data.id, lsu));
        
        // Add timeout event for the restored LSU
        int64_t timeoutTime_us = adjustedLastConnection_us + LSU_TIMEOUT_US + LSU_TIMEOUT_PADDING_US;
        TimeoutEvent event = {data.id, timeoutTime_us};
        timeoutQueue.push(event);
        
        ESP_LOGI(LSU_MANAGER_TAG, "Restored LSU ID: %lu, TimeSlot: %lu, LastConnection: %lld (adjusted by %lld)", 
                 data.id, data.timeSlotInPeriod, adjustedLastConnection_us, timeOffset_us);
    }
    
    update_lsu_count(connectedLSUs.size());
    
    // Update the next ID counter to avoid conflicts
    updateNextIdCounter();
    
    return true;
}
