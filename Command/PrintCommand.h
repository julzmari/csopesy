#pragma once

#include "ICommand.h"
#include <string>

class PrintCommand : public ICommand {
    public:
        PrintCommand(int pid, const std::string& printText);
        void execute(int coreId) override;

    private:
        int processId;
        std::string toPrint;
};