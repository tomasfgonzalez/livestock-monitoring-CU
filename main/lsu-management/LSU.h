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

class LSU {
  private:
    uint32_t id;
    uint32_t timeSlotInPeriod;
    int64_t lastConnectionTime_us; // Microseconds since boot (can be negative)

  public:
    LSU(uint32_t lsuId, uint32_t timeSlotInPeriod);

    uint32_t getId() const {return id;};
    uint32_t getTimeSlotInPeriod() const {return timeSlotInPeriod;};
    int64_t getLastConnectionTime() { return lastConnectionTime_us; };
    void setLastConnectionTime(int64_t time_us) { lastConnectionTime_us = time_us; };
};

#endif /* LSU_H */
