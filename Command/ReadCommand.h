#pragma once
#include "Command.h"
#include <string>

class ReadCommand : public Command {
public:
    ReadCommand(const std::string& varName, uint16_t address);
    void execute(process& context) override;
    std::shared_ptr<Command> clone() const override {
        return std::make_shared<ReadCommand>(*this);
    }

private:
    std::string variableName;
    uint16_t memoryAddress;
};
