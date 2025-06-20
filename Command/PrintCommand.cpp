#include "PrintCommand.h"
#include <fstream>
#include <chrono>
#include <ctime>
#include <sstream>

PrintCommand::PrintCommand(uint16_t value) {
    toPrint = std::to_string(value);
}

PrintCommand::PrintCommand() : toPrint("") {
}

void PrintCommand::execute(process& context) {
    auto now = std::chrono::system_clock::now();
    std::time_t timestamp = std::chrono::system_clock::to_time_t(now);

    if (toPrint == "") {
		std::stringstream ss;
        ss << "[Core " << context.getPid() << "] "
            << std::put_time(std::localtime(&timestamp), "(%m/%d/%Y %H:%M:%S)") << ": "
            << "\"Hello world from " << context.getProcessName() << "!\"" << "\n";

		context.addLog(ss);
    }
    else {
        std::stringstream printCmd;
        printCmd << "[Core " << context.getPid() << "] "
            << std::put_time(std::localtime(&timestamp), "(%m/%d/%Y %H:%M:%S)") << ": "
            << "Value from: " << toPrint << "\n";

        context.addLog(printCmd);
    }
    
}