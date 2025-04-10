/**
  ******************************************************************************
  * @authors        : Tomas Gonzalez & Brian Morris
  * @file           : main.cpp
  * @brief          : Main program to test LSU management functionality
  ******************************************************************************
  */

/**
 * Run tests with the command:
 * g++ -std=c++17 testLSU.cpp LSU.cpp LSUManager.cpp -o testLSU && "./testLSU"
 */

#include <iostream>
#include <thread> 
#include <chrono>
#include "LSUManager.h"

// Function to display LSU information
void displayLSUInfo(LSU* lsu) {
    if (lsu) {
        std::cout << "LSU ID: " << lsu->getId() << std::endl;
        std::cout << "Last Connection Time: " << lsu->getLastConnectionTime() << std::endl;
        std::cout << "------------------------" << std::endl;
    } else {
        std::cout << "LSU not found!" << std::endl;
    }
}

int main() {
    std::cout << "LSU Management System Test" << std::endl;
    std::cout << "=========================" << std::endl;
    
    // Create LSU manager
    LSUManager manager;
    
    // Test creating LSUs
    std::cout << "Creating LSUs..." << std::endl;
    auto [lsu1, success1] = manager.createLSU();
    auto [lsu2, success2] = manager.createLSU();
    auto [lsu3, success3] = manager.createLSU();
    
    std::cout << "Number of LSUs: " << manager.getLSUCount() << std::endl;
    printf("LSU1: %s SUCCEDED\n", success1 ? "HAS" : "HASN'T");
    printf("LSU2: %s SUCCEDED\n", success2 ? "HAS" : "HASN'T");
    printf("LSU3: %s SUCCEDED\n", success3 ? "HAS" : "HASN'T");
    
    // Display LSU information
    if (lsu1) {
        std::cout << "LSU 1 created with ID: " << lsu1->getId() << std::endl;
        displayLSUInfo(lsu1);
    }
    
    if (lsu2) {
        std::cout << "LSU 2 created with ID: " << lsu2->getId() << std::endl;
        displayLSUInfo(lsu2);
    }
    
    if (lsu3) {
        std::cout << "LSU 3 created with ID: " << lsu3->getId() << std::endl;
        displayLSUInfo(lsu3);
    }
    
    // Test keepalive functionality
    std::cout << "Testing keepalive for LSU 1..." << std::endl;
    // Sleep for a second to ensure time difference
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    if (manager.keepaliveLSU(lsu1->getId())) {
        std::cout << "Keepalive successful for LSU 1" << std::endl;
        displayLSUInfo(manager.getLSU(lsu1->getId()));
    } else {
        std::cout << "Keepalive failed for LSU 1" << std::endl;
    }
    
    // Test removing an LSU
    std::cout << "Removing LSU 2..." << std::endl;
    if (manager.removeLSU(lsu2->getId())) {
        std::cout << "LSU 2 removed successfully" << std::endl;
    } else {
        std::cout << "Failed to remove LSU 2" << std::endl;
    }
    
    std::cout << "Number of LSUs after removal: " << manager.getLSUCount() << std::endl;
    
    // Test timeout processing
    std::cout << "Processing timeouts..." << std::endl;
    manager.processTimeouts();
    std::cout << "Number of LSUs after timeout processing: " << manager.getLSUCount() << std::endl;
    
    // Final check of all LSUs
    std::cout << "\nFinal LSU status:" << std::endl;
    displayLSUInfo(manager.getLSU(0)); // LSU 1
    displayLSUInfo(manager.getLSU(1)); // LSU 2 (should be removed)
    displayLSUInfo(manager.getLSU(2)); // LSU 3
    
    std::cout << "LSU Management System Test Completed" << std::endl;
    return 0;
}
