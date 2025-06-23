#pragma once

#include "Command.h"
#include "myProcess.h"
#include <string>

class AddCommand : public Command {
public:
	AddCommand(const std::string& var1, const std::string& var2, const std::string& var3);
	AddCommand(const std::string& var1, uint16_t value1, const std::string& operand2); //var 2 is value
	AddCommand(const std::string& var1, const std::string& operand1, uint16_t value2); //var 3 is value
	AddCommand(const std::string& var1, uint16_t value1, uint16_t value2); // both are value

	void execute(process& context) override;
	std::shared_ptr<Command> clone() const override {
		return std::make_shared<AddCommand>(*this);
	}

private:
	std::string targetVar;
	std::string op1;
	std::string op2;

	uint16_t findValue(const std::string& varName, process& context) const;
};