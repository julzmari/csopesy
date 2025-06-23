#pragma once

#include "Command.h"
#include <string>

class PrintCommand : public Command {
    public:
        explicit PrintCommand(uint16_t value);
		PrintCommand();
        void execute(process& context);

    private:
        std::string toPrint;
};