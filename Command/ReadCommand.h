#pragma once
#include "Command.h"
#include <string>

class ReadCommand : public Command {
public:
    ReadCommand(const std::string& varName, uint32_t memoryAddress);
    void execute(process& context) override;
private:
    std::string varName;
    uint32_t memoryAddress;
};
