#pragma once
#include "Command.h"
#include <string>

class WriteCommand : public Command {
public:
    WriteCommand(uint32_t memoryAddress, uint16_t value);
    void execute(process& context) override;
private:
    uint32_t memoryAddress;
    uint16_t value;
};
