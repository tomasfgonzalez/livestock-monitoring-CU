/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : LSU.h
  * @brief          : LSU class, stores data of a connected LSU in the system
  ******************************************************************************
  */

#ifndef LSU_H
#define LSU_H

#include <ctime>
#include <cstdint>

class LSU {
  private:
    uint32_t id;
    uint32_t timeSlotInPeriod;
    time_t lastConnectionTime;

  public:
    LSU(uint32_t lsuId, uint32_t timeSlotInPeriod);

    uint32_t getId() const {return id;};
    uint32_t getTimeSlotInPeriod() const {return timeSlotInPeriod;};
    time_t getLastConnectionTime() const { return lastConnectionTime; };
    void setLastConnectionTime(time_t time) { lastConnectionTime = time; };
};

#endif /* LSU_H */
