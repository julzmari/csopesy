#include "WriteCommand.h"
#include "myProcess.h"
#include <iostream>
#include <sstream>

WriteCommand::WriteCommand(uint16_t address, const std::string& varName)
    : memoryAddress(address), variableName(varName) {
}

void WriteCommand::execute(process& context) {
    // Get the value from the variable
    auto it = context.variables.find(variableName);
    if (it == context.variables.end()) {
        std::stringstream ss;
        ss << "WRITE ERROR: Variable '" << variableName << "' not found";
        context.addLog(ss);
        return;
    }
    
    uint16_t value = it->second;
    
    // For now, we'll simulate writing to memory
    // In a real implementation, this would access the MemoryManager
    context.getMemoryManager()->writeUint16(memoryAddress, value);
    
    std::stringstream ss;
    ss << "WRITE: " << value << " to address 0x" << std::hex << memoryAddress << std::dec;
    context.addLog(ss);
}