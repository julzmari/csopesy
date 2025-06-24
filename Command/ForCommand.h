#pragma once
#include "Command.h"
#include <vector>
#include <memory>

class ForCommand : public Command {
public:
	ForCommand(const std::vector<std::shared_ptr<Command>>& instructions, int repeats);
	void execute(process& context);
public:
	std::vector<std::shared_ptr<Command>> instructions;
	int repeats;
};