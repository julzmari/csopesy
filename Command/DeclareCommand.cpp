#include "DeclareCommand.h"
#include "myProcess.h"

DeclareCommand::DeclareCommand(const std::string& varName, uint16_t value)
    : variableName(varName), variableValue(value) {
}

void DeclareCommand::execute(process& context) {
    if (context.variables.find(variableName) == context.variables.end()) {
        context.variables[variableName] = variableValue;
    }
}