/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : LSU.h
  * @brief          : LSU class, stores data of a connected LSU in the system
  ******************************************************************************
  */

#ifndef LSU_H
#define LSU_H

#include <cstdint>
#include "esp_timer.h"

class LSU {
  private:
    uint32_t id;
    uint32_t timeSlotInPeriod;
    uint64_t lastConnectionTime_us;

  public:
    LSU(uint32_t lsuId, uint32_t timeSlotInPeriod);

    uint32_t getId() const {return id;};
    uint32_t getTimeSlotInPeriod() const {return timeSlotInPeriod;};
    uint64_t getLastConnectionTime() { return lastConnectionTime_us; };
    void setLastConnectionTime(uint64_t time) { lastConnectionTime_us = time; };
};

#endif /* LSU_H */
