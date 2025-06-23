#pragma once

#include "Command.h"
#include "myProcess.h"
#include <string>

class SubtractCommand : public Command {
public:
	SubtractCommand(const std::string& var1, uint16_t value1, const std::string& operand2); //var 2 is value
	SubtractCommand(const std::string& var1, const std::string& var2, const std::string& var3);
	SubtractCommand(const std::string& var1, const std::string& operand1, uint16_t value2); //var 3 is value
	SubtractCommand(const std::string& var1, uint16_t value1, uint16_t value2); // both are value

	void execute(process& context) override;

private:
	std::string targetVar;
	std::string op1;
	std::string op2;

	uint16_t findValue(const std::string& varName, process& context) const;
};