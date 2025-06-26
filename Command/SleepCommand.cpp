#include "SleepCommand.h"
#include "myProcess.h"
#ifdef _WIN32
#include <windows.h>
#endif

SleepCommand::SleepCommand(uint8_t sleepTime)
    : sleepTime(sleepTime) {
}

void SleepCommand::execute(process& context) {
	context.setSleeping(true);
	context.setSleepTime(sleepTime);
}