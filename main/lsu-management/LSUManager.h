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
#include "nvs.h"
#include "nvs_flash.h"

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
    
    // NVS storage keys
    static const char* NVS_NAMESPACE;
    static const char* NVS_LSU_COUNT_KEY;
    static const char* NVS_LSU_DATA_KEY;
    
    /**
     * @brief Generates a unique ID for a new LSU
     * @return Unique LSU identifier
     */
    uint32_t generateLSUId();

    uint32_t generateTimeSlot();

    /**
     * @brief Serializes LSU data for NVS storage
     * @param buffer Output buffer for serialized data
     * @param bufferSize Size of the buffer
     * @return Number of bytes written
     */
    size_t serializeLSUData(uint8_t* buffer, size_t bufferSize) const;

    /**
     * @brief Deserializes LSU data from NVS storage
     * @param buffer Input buffer with serialized data
     * @param dataSize Size of the data to deserialize
     * @return true if deserialization was successful
     */
    bool deserializeLSUData(const uint8_t* buffer, size_t dataSize);

    /**
     * @brief Saves LSU data to NVS
     * @return true if save was successful
     */
    bool saveToNVS();

    /**
     * @brief Loads LSU data from NVS
     * @return true if load was successful
     */
    bool loadFromNVS();

  public:
    LSUManager();

    /**
     * @brief Destructor - saves data before cleanup
     */
    ~LSUManager();

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
