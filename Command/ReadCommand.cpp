#include "ReadCommand.h"  
#include "MemoryManager.h"  
#include "myProcess.h"
#include <stdexcept>
  
ReadCommand::ReadCommand(const std::string& varName, uint32_t memoryAddress)  
    : varName(varName), memoryAddress(memoryAddress) {}  
  
void ReadCommand::execute(process& context) {  
    if (context.variables.size() >= 32 && context.variables.find(varName) == context.variables.end())  
        return;  
  
    if (!context.memoryManager.isValidAddress(memoryAddress)) {  
        throw std::runtime_error("Access violation: Invalid memory address");  
    }  
  
    uint16_t value = context.memoryManager.readUint16(memoryAddress);  
    context.variables[varName] = value;  
}