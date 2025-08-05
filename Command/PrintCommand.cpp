#include "PrintCommand.h"
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>
#include "myProcess.h"
#include <thread>
#include <iostream>
#include <iomanip>
#include <algorithm>

PrintCommand::PrintCommand(uint16_t value) {
    toPrint = std::to_string(value);
}

PrintCommand::PrintCommand() : toPrint("") {}

PrintCommand::PrintCommand(const std::string& msg) {
    toPrint = msg;
}

void PrintCommand::execute(process& context) {
    auto now = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(now);

    std::string evaluated = toPrint;
    std::stringstream ss(evaluated);
    std::string segment;
    std::string finalMsg;

    while (std::getline(ss, segment, '+')) {
        segment.erase(0, segment.find_first_not_of(" \t"));
        segment.erase(segment.find_last_not_of(" \t") + 1);

        segment.erase(std::remove(segment.begin(), segment.end(), '\\'), segment.end());

        if (segment.size() >= 2 && segment.front() == '"' && segment.back() == '"') {
            segment = segment.substr(1, segment.size() - 2);
            finalMsg += segment;
        } else {
            auto it = context.variables.find(segment);
            if (it != context.variables.end()) {
                finalMsg += std::to_string(it->second);
            } else {
                finalMsg += "";  
            }
        }
    }

    // Create the log line
    std::stringstream logLine;
    logLine << "[Core " << context.getCoreId() << "] "
            << std::put_time(std::localtime(&timestamp), "(%m/%d/%Y %H:%M:%S)") 
            << ": " << finalMsg << "\n";

    context.addLog(logLine);

    if (!finalMsg.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        std::cout << "\n" << logLine.str() << std::flush;
        std::cout << "Enter command: " << std::flush;

    }

    if (delayTime > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(delayTime));
    }
}
