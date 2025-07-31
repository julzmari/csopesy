#include "ReadCommand.h"
#include "myProcess.h"
#include <iostream>
#include <sstream>

ReadCommand::ReadCommand(const std::string& varName, uint16_t address)
    : variableName(varName), memoryAddress(address) {
}

void ReadCommand::execute(process& context) {
    uint16_t val = context.getMemoryManager()->readUint16(memoryAddress);
    context.variables[variableName] = val;

    std::stringstream ss;
    ss << "READ: " << variableName << " = " << val 
       << " from address 0x" << std::hex << memoryAddress << std::dec;
    context.addLog(ss);
}