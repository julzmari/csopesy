#include "ForCommand.h"
#include "myProcess.h"

ForCommand::ForCommand(const std::vector<std::shared_ptr<Command>>& instructions, int repeats)
    : instructions(instructions), repeats(repeats) {
}

void ForCommand::execute(process& context) {
    int insertPos = context.getCurrentLine();
    std::vector<std::shared_ptr<Command>> toInsert;
    for (int i = 0; i < repeats; ++i) {
        for (const auto& cmd : instructions) {
            toInsert.push_back(cmd->clone());
        }
    }
    context.insertInstructions(insertPos, toInsert);
    context.setCurrentLine(insertPos + toInsert.size());
}

std::shared_ptr<Command> ForCommand::clone() const {
    std::vector<std::shared_ptr<Command>> copiedInstructions;
    for (const auto& cmd : instructions) {
        copiedInstructions.push_back(cmd->clone());
    }
    return std::make_shared<ForCommand>(copiedInstructions, repeats);
}
