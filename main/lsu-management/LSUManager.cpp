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
#include "LSU.h"
#include "general_config.h"
#include "esp_log.h"

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
    
    if (result.second) {
        // Add timeout event for the new LSU
        TimeoutEvent event = {lsuId, time(nullptr) + DEFAULT_TIMEOUT_MS};
        timeoutQueue.push(event);
        return std::make_pair(newLSU, true);
    }
    
    return std::make_pair(nullptr, false); // Insert failed
}

bool LSUManager::removeLSU(uint32_t lsuId) {
    auto it = connectedLSUs.find(lsuId);
    if (it != connectedLSUs.end()) {
        connectedLSUs.erase(it);
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
        return true;
    }
    return false; // LSU not found
}

void LSUManager::processTimeouts() {
    time_t currentTime = time(nullptr);

    while (!timeoutQueue.empty() && timeoutQueue.top().timeoutTime <= currentTime) {
        TimeoutEvent event = timeoutQueue.top();
        timeoutQueue.pop();
        
        LSU* lsu = getLSU(event.lsuId);
        if (lsu != nullptr) {
            if (lsu->getLastConnectionTime() + DEFAULT_TIMEOUT_MS <= currentTime) {
                // LSU has timed out, remove it
                connectedLSUs.erase(event.lsuId);
            }
        }
    }
}
