#pragma once

#include "Command.h"
#include <string>

class PrintCommand : public Command {
    public:
        explicit PrintCommand(uint16_t value);
		PrintCommand();
        PrintCommand(const std::string& msg, int delay);
        void execute(process& context);
        std::shared_ptr<Command> clone() const override {
            return std::make_shared<PrintCommand>(*this);
        }

    private:
        std::string toPrint;
        int delayTime = 0;
};