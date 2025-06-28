#include "ForCommand.h"
#include "myProcess.h"

ForCommand::ForCommand(const std::vector<std::shared_ptr<Command>>& instructions, int repeats)
    : instructions(instructions), repeats(repeats) {
}

void ForCommand::execute(process& context) {
    if (loopPosition == -1) {
        loopPosition = context.getCurrentLine();
        currentIteration = 0;
    }

    if (currentIteration < repeats) {
        for (const auto& cmd : instructions) {
            cmd->execute(context);
        }
        ++currentIteration;
        context.setCurrentLine(loopPosition);
    }
    else {
        loopPosition = -1;
        currentIteration = 0;
        context.setCurrentLine(loopPosition + 1);
    }
}

std::shared_ptr<Command> ForCommand::clone() const {
    std::vector<std::shared_ptr<Command>> copiedInstructions;
    for (const auto& cmd : instructions) {
        copiedInstructions.push_back(cmd->clone());
    }
    return std::make_shared<ForCommand>(copiedInstructions, repeats);
}
