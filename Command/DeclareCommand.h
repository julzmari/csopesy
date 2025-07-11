#pragma once
#include <string>
#include "Command.h"

class DeclareCommand : public Command {
public:
	DeclareCommand(const std::string& varName, uint16_t value);
	void execute(process& context) override;

	std::shared_ptr<Command> clone() const override {
		return std::make_shared<DeclareCommand>(*this);
	}

private:
	std::string variableName;
	uint16_t variableValue;
};