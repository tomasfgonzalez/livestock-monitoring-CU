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
    time_t lastConnectionTime;

  public:
    LSU(uint32_t lsuId);
    
    uint32_t getId() const {return id;};
    time_t getLastConnectionTime() { return lastConnectionTime; };
    void setLastConnectionTime(time_t time) { lastConnectionTime = time; };
};

#endif /* LSU_H */
