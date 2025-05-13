/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : LSU.cpp
  * @brief          : LSU class, stores data of a connected LSU in the system
  ******************************************************************************
  */

#include "LSU.h"

LSU::LSU(uint32_t lsuId, uint32_t timeSlotInPeriod) : id(lsuId), timeSlotInPeriod(timeSlotInPeriod) {
  lastConnectionTime = time(nullptr);
}
