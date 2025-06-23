#pragma once
#include "Command.h"
#include "myProcess.h"
#include <cstdint>

class SleepCommand : public Command {
public:
	virtual void execute(process& context) const;
	SleepCommand(uint8_t sleepTime);
private:
	uint8_t sleepTime;
};