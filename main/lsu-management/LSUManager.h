/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : LSUManager.h
  * @brief          : LSUManager class, manages connected LSUs in the system
  ******************************************************************************
  */

#ifndef LSU_MANAGER_H
#define LSU_MANAGER_H

/* Includes ------------------------------------------------------------------*/
#include <map>
#include <queue>
#include <vector>
#include <functional>
#include <cstdint>
#include "esp_timer.h"
#include "LSU.h"
#include "general_config.h"

/* Macros -------------------------------------------------------------------*/
#define LSU_TIMEOUT_US (2 * TIME_PERIOD_MS * 1000)  // Two whole periods in microseconds
#define LSU_TIMEOUT_PADDING_US 1000000 // 1 second

/* Structs -------------------------------------------------------------------*/
struct TimeoutEvent {
  uint32_t lsuId;
  int64_t timeoutTime; // microseconds since boot when timeout occurs
  
  bool operator>(const TimeoutEvent& other) const {
    return timeoutTime > other.timeoutTime;
  }
};

struct LSUData {
  uint32_t id;
  uint32_t timeSlotInPeriod;
  int64_t lastConnectionTime_us; // Microseconds since boot (can be negative)
};

/* Typedefs ------------------------------------------------------------------*/
typedef std::map<uint32_t, LSU*> LSUMap;
typedef std::priority_queue<TimeoutEvent, std::vector<TimeoutEvent>, std::greater<TimeoutEvent>> TimeoutQueue;

/* Class ---------------------------------------------------------------------*/
class LSUManager {
  private:
    LSUMap connectedLSUs;
    TimeoutQueue timeoutQueue;
    uint32_t nextLSUId;
    
    /**
     * @brief Generates a unique ID for a new LSU
     * @return Unique LSU identifier
     */
    uint32_t generateLSUId();

    /**
     * @brief Generates a time slot for a new LSU
     * @return Time slot
     */
    uint32_t generateTimeSlot();

    /**
     * @brief Updates the next ID counter based on loaded LSUs
     */
    void updateNextIdCounter();

  public:
    LSUManager() : nextLSUId(0x02) {} // Start at 0x02 to avoid conflict with CU

    /**
     * @brief Creates and adds a new LSU to the system
     * @return true if LSU was added successfully, false otherwise
     */
    std::pair<LSU*, bool> createLSU();

    /**
     * @brief Removes an LSU from the system by its ID
     * @param lsuId The ID of the LSU to remove
     * @return true if LSU was removed successfully, false if not found
     */
    bool removeLSU(uint32_t lsuId);

    /**
     * @brief Gets an LSU by its ID
     * @param lsuId The ID of the LSU to retrieve
     * @return Pointer to the LSU if found, nullptr otherwise
     */
    LSU* getLSU(uint32_t lsuId);

    /**
     * @brief Updates the last connection time for an LSU
     * @param lsuId The ID of the LSU to update
     * @return true if LSU was updated successfully, false if not found
     */
    bool keepaliveLSU(uint32_t lsuId);

    /**
     * @brief Processes timeout events and removes timed-out LSUs
     */
    void processTimeouts();

    /**
     * @brief Gets the number of LSUs currently managed
     * @return The count of connected LSUs
     */
    size_t getLSUCount() const { return connectedLSUs.size(); }

    /**
     * @brief Gets a list of all LSU data for debugging/monitoring
     * @return Vector of LSUData containing all LSU information
     */
    std::vector<LSUData> getLsuSerializedData() const;

    /**
     * @brief Restores LSUs from serialized data (for NVS persistence)
     * @param lsuDataVector Vector of LSUData to restore
     * @param savedTimestamp Timestamp when data was saved (for time offset calculation)
     * @return true if restore was successful, false otherwise
     */
    bool restoreLsuFromSerializedData(const std::vector<LSUData>& lsuDataVector, int64_t savedTimestamp_us = 0);
};

#endif /* LSU_MANAGER_H */
