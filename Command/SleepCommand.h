#pragma once
#include "Command.h"
#include "myProcess.h"
#include <cstdint>

class SleepCommand : public Command {
public:
	virtual void execute(process& context) const;
	SleepCommand(uint8_t sleepTime);

	std::shared_ptr<Command> clone() const override {
		return std::make_shared<SleepCommand>(*this);
	}
private:
	uint8_t sleepTime;
};