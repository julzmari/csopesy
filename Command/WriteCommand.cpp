#include "WriteCommand.h"
#include "MemoryManager.h"
#include "myProcess.h"
#include <stdexcept>
#include <algorithm>

WriteCommand::WriteCommand(uint32_t memoryAddress, uint16_t value)
    : memoryAddress(memoryAddress), value(value) {}

void WriteCommand::execute(process& context) {
    if (!context.memoryManager.isValidAddress(memoryAddress)) {
        throw std::runtime_error("Access violation: Invalid memory address");
    }

    uint16_t clampedValue = std::clamp(value, static_cast<uint16_t>(0), std::numeric_limits<uint16_t>::max());
    context.memoryManager.writeUint16(memoryAddress, clampedValue);
}