#include "AddCommand.h"

AddCommand::AddCommand(const std::string& var1, const std::string& var2, const std::string& var3)
    : targetVar(var1), op1(var1), op2(var3) {
}

AddCommand::AddCommand(const std::string& var1, uint16_t value1, const std::string& operand2)
    : AddCommand(var1, std::to_string(value1), operand2) {
}

AddCommand::AddCommand(const std::string& var1, const std::string& operand1, uint16_t value2)
    : AddCommand(var1, operand1, std::to_string(value2)) {
}

AddCommand::AddCommand(const std::string& var1, uint16_t value1, uint16_t value2)
    : AddCommand(var1, std::to_string(value1), std::to_string(value2)) {
}

void AddCommand::execute(process& context) {
    uint16_t value1 = findValue(op1, context);
    uint16_t value2 = findValue(op2, context);

    // Always overwrite the target variable with the sum
    context.variables[targetVar] = value1 + value2;

    // Optional: Log the result for debugging
    std::stringstream ss;
    ss << "ADD: " << targetVar << " = " << value1 << " + " << value2
       << " = " << context.variables[targetVar];
    context.addLog(ss);
}

uint16_t AddCommand::findValue(const std::string& varName, process& context) const {
    if (isdigit(varName[0])) {
        return static_cast<uint16_t>(std::stoi(varName));
    } else {
        auto it = context.variables.find(varName);
        if (it != context.variables.end()) {
            return it->second;
        }
    }
}