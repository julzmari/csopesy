#pragma once
#include "Command.h"
#include <string>

class WriteCommand : public Command {
public:
    WriteCommand(uint16_t address, const std::string& varName);
    void execute(process& context) override;
    std::shared_ptr<Command> clone() const override {
        return std::make_shared<WriteCommand>(*this);
    }

private:
    uint16_t memoryAddress;
    std::string variableName;
};
