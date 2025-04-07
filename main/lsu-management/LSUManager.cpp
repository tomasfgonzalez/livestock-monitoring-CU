/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : LSUManager.h
  * @brief          : LSUManager class, manages connected LSUs in the system
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "LSUManager.h"
#include <iostream>

/* Helper functions ----------------------------------------------------------*/
uint32_t LSUManager::generateLSUId() {
    static uint32_t nextId = 0;
    std::cout << "Generating LSU ID: " << nextId << std::endl;
    return nextId++;
}

/* Function implementations -------------------------------------------------*/
std::pair<LSU*, bool> LSUManager::createLSU() {
    uint32_t lsuId = generateLSUId();
    LSU* newLSU = new LSU(lsuId);
    
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
