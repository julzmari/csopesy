#include "SubtractCommand.h"

SubtractCommand::SubtractCommand(const std::string& var1, const std::string& var2, const std::string& var3)
    : targetVar(var1), op1(var1), op2(var3) {
}

SubtractCommand::SubtractCommand(const std::string& var1, uint16_t value1, const std::string& operand2)
    : SubtractCommand(var1, std::to_string(value1), operand2) {
}

SubtractCommand::SubtractCommand(const std::string& var1, const std::string& operand1, uint16_t value2)
    : SubtractCommand(var1, operand1, std::to_string(value2)) {
}

SubtractCommand::SubtractCommand(const std::string& var1, uint16_t value1, uint16_t value2)
    : SubtractCommand(var1, std::to_string(value1), std::to_string(value2)) {
}

void SubtractCommand::execute(process& context) {
    uint16_t value1 = findValue(op1, context);
    uint16_t value2 = findValue(op2, context);
    if (context.variables.find(targetVar) == context.variables.end()) { //if not exists
        context.variables[targetVar] = value1 - value2;
    }
    else {
        context.variables[targetVar] -= value1 - value2;
    }
}

uint16_t SubtractCommand::findValue(const std::string& varName, process& context) const {
    // if string is digit store new var return digit
    if (isdigit(varName[0])) {
        std::string key;
        key = "var_" + varName ;
        context.variables[key] = static_cast<uint16_t>(std::stoi(varName));
        return static_cast<uint16_t>(std::stoi(varName));
    }
    else {
        auto it = context.variables.find(varName);
        if (it != context.variables.end()) {
            return it->second;
        }
    }
}