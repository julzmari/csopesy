#include "PrintCommand.h"
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>

PrintCommand::PrintCommand(int pid, const std::string& printText)
    : processId(pid), toPrint(printText) {
}

void PrintCommand::execute(int coreId) {
    std::ostringstream filename;
    filename << "Process_" << processId << ".txt";

    std::ofstream outFile(filename.str(), std::ios::app);

    auto now = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(now);

    outFile << "[Core " << coreId << "] "
        << std::put_time(std::localtime(&timestamp), "(%m/%d/%Y %H:%M:%S)") << ": "
        << toPrint << "\n";
}