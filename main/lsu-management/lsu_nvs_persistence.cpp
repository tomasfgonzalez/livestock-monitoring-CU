/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : lsu_nvs_persistence.cpp
  * @brief          : NVS persistence module for LSUManager data
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "lsu_nvs_persistence.h"
#include "LSUManager.h"
#include "LSU.h"
#include "general_config.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs.h"
#include "nvs_flash.h"

/* Private variables --------------------------------------------------------- */
static const char *LSU_NVS_TAG = "LSU NVS";

/* Function implementations --------------------------------------------------*/
bool lsu_nvs_save(LSUManager& manager) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS handle
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_NVS_TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return false;
    }
    
    // Get LSU data from manager
    std::vector<LSUData> lsuDataVector = manager.getLsuSerializedData();
    
    // Save timestamp snapshot (in microseconds since boot)
    int64_t currentTime_us = esp_timer_get_time();
    err = nvs_set_i64(nvs_handle, NVS_TIMESTAMP_KEY, currentTime_us);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_NVS_TAG, "Error saving timestamp to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }
    
    // Save LSU count
    uint32_t lsuCount = lsuDataVector.size();
    err = nvs_set_u32(nvs_handle, NVS_LSU_COUNT_KEY, lsuCount);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_NVS_TAG, "Error saving LSU count to NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }
    
    // Save LSU data
    size_t dataSize = lsuDataVector.size() * sizeof(LSUData);
    if (dataSize > 0) {
        err = nvs_set_blob(nvs_handle, NVS_LSU_DATA_KEY, lsuDataVector.data(), dataSize);
        if (err != ESP_OK) {
            ESP_LOGE(LSU_NVS_TAG, "Error saving LSU data to NVS: %s", esp_err_to_name(err));
            nvs_close(nvs_handle);
            return false;
        }
    }
    
    // Commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_NVS_TAG, "Error committing NVS changes: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }
    
    nvs_close(nvs_handle);
    ESP_LOGI(LSU_NVS_TAG, "Successfully saved %lu LSUs to NVS", lsuCount);
    return true;
}

bool lsu_nvs_load(LSUManager& manager) {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS handle
    err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_NVS_TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return false;
    }
    
    // Get timestamp snapshot (in microseconds since boot)
    int64_t savedTimestamp_us = 0;
    err = nvs_get_i64(nvs_handle, NVS_TIMESTAMP_KEY, &savedTimestamp_us);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGI(LSU_NVS_TAG, "No timestamp found in NVS (first boot)");
        } else {
            ESP_LOGW(LSU_NVS_TAG, "Error reading timestamp from NVS: %s", esp_err_to_name(err));
        }
        nvs_close(nvs_handle);
        return false;
    }
    
    // Get LSU count
    uint32_t lsuCount = 0;
    err = nvs_get_u32(nvs_handle, NVS_LSU_COUNT_KEY, &lsuCount);
    if (err != ESP_OK) {
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGI(LSU_NVS_TAG, "No LSU data found in NVS (first boot)");
        } else {
            ESP_LOGW(LSU_NVS_TAG, "Error reading LSU count from NVS: %s", esp_err_to_name(err));
        }
        nvs_close(nvs_handle);
        return false;
    }
    
    if (lsuCount == 0) {
        ESP_LOGI(LSU_NVS_TAG, "No LSUs to load from NVS");
        nvs_close(nvs_handle);
        return true;
    }
    
    // Validate LSU count is reasonable
    if (lsuCount > MAX_LSU_COUNT) {
        ESP_LOGE(LSU_NVS_TAG, "Invalid LSU count in NVS: %lu (max: %d)", lsuCount, MAX_LSU_COUNT);
        nvs_close(nvs_handle);
        return false;
    }
    
    // Get LSU data
    std::vector<LSUData> lsuDataVector(lsuCount);
    size_t dataSize = lsuCount * sizeof(LSUData);
    err = nvs_get_blob(nvs_handle, NVS_LSU_DATA_KEY, lsuDataVector.data(), &dataSize);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_NVS_TAG, "Error loading LSU data from NVS: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }
    
    nvs_close(nvs_handle);
    
    // Restore LSUs to manager with timestamp offset
    if (!manager.restoreLsuFromSerializedData(lsuDataVector, savedTimestamp_us)) {
        ESP_LOGE(LSU_NVS_TAG, "Failed to restore LSUs to manager");
        return false;
    }
    
    ESP_LOGI(LSU_NVS_TAG, "Successfully loaded %lu LSUs from NVS (saved at timestamp %lld us)", lsuCount, savedTimestamp_us);
    return true;
}

bool lsu_nvs_clear() {
    nvs_handle_t nvs_handle;
    esp_err_t err;
    
    // Open NVS handle
    err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_NVS_TAG, "Error opening NVS handle: %s", esp_err_to_name(err));
        return false;
    }
    
    // Erase all keys in the namespace
    err = nvs_erase_all(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_NVS_TAG, "Error erasing NVS data: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }
    
    // Commit changes
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        ESP_LOGE(LSU_NVS_TAG, "Error committing NVS changes: %s", esp_err_to_name(err));
        nvs_close(nvs_handle);
        return false;
    }
    
    nvs_close(nvs_handle);
    ESP_LOGI(LSU_NVS_TAG, "Successfully cleared LSU data from NVS");
    return true;
}

bool lsu_nvs_test(LSUManager& manager) {
    ESP_LOGI(LSU_NVS_TAG, "Starting comprehensive NVS functionality test");
    
    // Store original LSU count
    size_t originalCount = manager.getLSUCount();
    ESP_LOGI(LSU_NVS_TAG, "Original LSU count: %zu", originalCount);
    
    // Create test LSUs
    auto [testLSU1, success1] = manager.createLSU();
    if (!success1) {
        ESP_LOGE(LSU_NVS_TAG, "Failed to create first test LSU");
        return false;
    }
    
    auto [testLSU2, success2] = manager.createLSU();
    if (!success2) {
        ESP_LOGE(LSU_NVS_TAG, "Failed to create second test LSU");
        return false;
    }
    
    uint32_t testId1 = testLSU1->getId();
    uint32_t testId2 = testLSU2->getId();
    uint32_t testTimeSlot1 = testLSU1->getTimeSlotInPeriod();
    uint32_t testTimeSlot2 = testLSU2->getTimeSlotInPeriod();
    uint64_t testLastConnection1 = testLSU1->getLastConnectionTime();
    uint64_t testLastConnection2 = testLSU2->getLastConnectionTime();
    
    ESP_LOGI(LSU_NVS_TAG, "Created test LSUs - ID1: %lu (slot: %lu), ID2: %lu (slot: %lu)", 
             testId1, testTimeSlot1, testId2, testTimeSlot2);
    
    // Save to NVS
    if (!lsu_nvs_save(manager)) {
        ESP_LOGE(LSU_NVS_TAG, "Failed to save test data to NVS");
        return false;
    }
    
    ESP_LOGI(LSU_NVS_TAG, "Test data saved to NVS successfully");
    
    // Clear current LSUs to simulate a fresh boot
    manager.restoreLsuFromSerializedData({}); // Clear all LSUs
    ESP_LOGI(LSU_NVS_TAG, "Cleared LSUs, attempting to restore from NVS");
    
    // Load from NVS
    if (!lsu_nvs_load(manager)) {
        ESP_LOGE(LSU_NVS_TAG, "Failed to load test data from NVS");
        return false;
    }
    
    // Verify the LSUs were restored correctly
    LSU* restoredLSU1 = manager.getLSU(testId1);
    LSU* restoredLSU2 = manager.getLSU(testId2);
    
    if (restoredLSU1 == nullptr) {
        ESP_LOGE(LSU_NVS_TAG, "Test LSU1 not found after restore");
        return false;
    }
    
    if (restoredLSU2 == nullptr) {
        ESP_LOGE(LSU_NVS_TAG, "Test LSU2 not found after restore");
        return false;
    }
    
    // Verify LSU1 data
    if (restoredLSU1->getTimeSlotInPeriod() != testTimeSlot1) {
        ESP_LOGE(LSU_NVS_TAG, "LSU1 time slot mismatch - expected: %lu, got: %lu", 
                 testTimeSlot1, restoredLSU1->getTimeSlotInPeriod());
        return false;
    }
    
    if (restoredLSU1->getLastConnectionTime() != testLastConnection1) {
        ESP_LOGE(LSU_NVS_TAG, "LSU1 last connection time mismatch - expected: %llu, got: %llu", 
                 testLastConnection1, restoredLSU1->getLastConnectionTime());
        return false;
    }
    
    // Verify LSU2 data
    if (restoredLSU2->getTimeSlotInPeriod() != testTimeSlot2) {
        ESP_LOGE(LSU_NVS_TAG, "LSU2 time slot mismatch - expected: %lu, got: %lu", 
                 testTimeSlot2, restoredLSU2->getTimeSlotInPeriod());
        return false;
    }
    
    if (restoredLSU2->getLastConnectionTime() != testLastConnection2) {
        ESP_LOGE(LSU_NVS_TAG, "LSU2 last connection time mismatch - expected: %llu, got: %llu", 
                 testLastConnection2, restoredLSU2->getLastConnectionTime());
        return false;
    }
    
    // Verify total count
    size_t restoredCount = manager.getLSUCount();
    if (restoredCount != originalCount + 2) {
        ESP_LOGE(LSU_NVS_TAG, "LSU count mismatch - expected: %zu, got: %zu", 
                 originalCount + 2, restoredCount);
        return false;
    }
    
    ESP_LOGI(LSU_NVS_TAG, "NVS functionality test PASSED - all data verified");
    ESP_LOGI(LSU_NVS_TAG, "Test summary: %zu original + 2 test LSUs = %zu total", 
             originalCount, restoredCount);
    
    return true;
}
