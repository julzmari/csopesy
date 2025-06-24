#include "SleepCommand.h"
#include "myProcess.h"
#include <windows.h>

SleepCommand::SleepCommand(uint8_t sleepTime)
    : sleepTime(sleepTime) {
}

void SleepCommand::execute(process& context) {
	context.setSleeping(true);
	context.setSleepTime(sleepTime);
}