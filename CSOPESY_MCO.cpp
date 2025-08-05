#include <iostream>
#include <string>
#include "Config.h"
#include "OsEmulator.h"
#include <fstream>
#include <filesystem>
#include <thread>
#include <chrono>

using namespace std;

int main() {
    cout << "Program Start\n" << endl;

    Config config("Config/config.txt");

    string command;
    while (true) {
        std::cout << std::endl << "Enter command: " << std::flush; 
        std::getline(std::cin, command);
    
        trimSpaces(command);
    
        if (command == "initialize") {
            startEmulator(config);
            
        } else if (command == "exit") {
            std::cout << "Exiting program..." << std::endl;
            break;
        } else {
            std::cout << "Unknown command. Please try again." << std::endl;
        }
    }

    return 0;
}
