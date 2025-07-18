#pragma once
#include "Command.h"
#include <vector>
#include <memory>

class ForCommand : public Command
{
public:
	ForCommand(const std::vector<std::shared_ptr<Command>> &instructions, int repeats);
	void execute(process &context) override;
	int getRepeatCount() const { return repeats; }
	const std::vector<std::shared_ptr<Command>> &getInstructions() const { return instructions; }
	std::shared_ptr<Command> clone() const override;

public:
	std::vector<std::shared_ptr<Command>> instructions;
	int repeats;
	int currentIteration = 0;
	int loopPosition = -1;
};