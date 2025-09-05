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
#include "LSU.h"
#include "general_config.h"
#include "esp_log.h"
#include "display/status.h"

/* Private variables --------------------------------------------------------- */
static const char *LSU_MANAGER_TAG = "LSU Manager";

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
        uint64_t timeoutTime_us = esp_timer_get_time() + (LSU_TIMEOUT_MS + LSU_TIMEOUT_PADDING_MS) * 1000;
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
        std::string topic = "livestock/" + std::to_string(lsu_id);
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
        uint64_t currentTime_us = esp_timer_get_time();
        lsu->setLastConnectionTime(currentTime_us);

        uint64_t timeoutTime_us = currentTime_us + (LSU_TIMEOUT_MS + LSU_TIMEOUT_PADDING_MS) * 1000;
        TimeoutEvent event = {lsuId, timeoutTime_us};
        timeoutQueue.push(event);
        return true;
    }
    return false; // LSU not found
}

void LSUManager::processTimeouts() {
    uint64_t currentTime = esp_timer_get_time();

    while (!timeoutQueue.empty() && timeoutQueue.top().timeoutTime <= currentTime) {
        TimeoutEvent event = timeoutQueue.top();
        timeoutQueue.pop();
        
        LSU* lsu = getLSU(event.lsuId);
        if (lsu != nullptr) {
            // Convert LSU_TIMEOUT_MS to microseconds for comparison
            if (lsu->getLastConnectionTime() + (LSU_TIMEOUT_MS * 1000) <= currentTime) {
                // LSU has timed out, send alert and remove it
                uint32_t lsu_id = lsu->getId();
                
                // Send timeout alert to MQTT
                std::string topic = "livestock/" + std::to_string(lsu_id);
                std::string payload = "ALERT: Device timeout - LSU " + std::to_string(lsu_id) + 
                                      " has not communicated for " + std::to_string(LSU_TIMEOUT_MS/1000) + " seconds";
                extern void mqtt_api_publish(const char *topic, const char *payload);
                mqtt_api_publish(topic.c_str(), payload.c_str());
                
                ESP_LOGW(LSU_MANAGER_TAG, "LSU %lu timed out after %lu seconds - removed and alert sent", 
                         lsu_id, (uint32_t) LSU_TIMEOUT_MS/1000);
                
                // Remove the LSU
                connectedLSUs.erase(event.lsuId);
                update_lsu_count(connectedLSUs.size());
            }
        }
    }
}
