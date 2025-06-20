#pragma once
#include "Command.h"

class DeclareCommand : public Command {
public:
	DeclareCommand(const std::string& varName, uint16_t value)
		: variableName(varName), variableValue(value) {
	}
	void execute(process& context) override;

private:
	std::string variableName;
	uint16_t variableValue;
};