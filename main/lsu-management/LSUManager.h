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
#include <ctime>
#include <cstdint>
#include "LSU.h"

/* Macros -------------------------------------------------------------------*/
#define DEFAULT_TIMEOUT_MS 5000

/* Structs -------------------------------------------------------------------*/
struct TimeoutEvent {
  uint32_t lsuId;
  time_t timeoutTime;
  
  bool operator>(const TimeoutEvent& other) const {
    return timeoutTime > other.timeoutTime;
  }
};

/* Typedefs ------------------------------------------------------------------*/
typedef std::map<uint32_t, LSU*> LSUMap;
typedef std::priority_queue<TimeoutEvent, std::vector<TimeoutEvent>, std::greater<TimeoutEvent>> TimeoutQueue;

/* Class ---------------------------------------------------------------------*/
class LSUManager {
  private:
    LSUMap connectedLSUs;
    TimeoutQueue timeoutQueue;
    
    /**
     * @brief Generates a unique ID for a new LSU
     * @return Unique LSU identifier
     */
    uint32_t generateLSUId();

    uint32_t generateTimeSlot();

  public:
    LSUManager() {}

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
};

#endif /* LSU_MANAGER_H */
