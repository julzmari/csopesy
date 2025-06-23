#include "ForCommand.h"
#include "myProcess.h"

ForCommand::ForCommand(const std::vector<std::shared_ptr<Command>>& instructions, int repeats)
    : instructions(instructions), repeats(repeats) {
}

void ForCommand::execute(process& context) {
    for (currentRepeat = 0; currentRepeat < repeats; ++currentRepeat) {
        for (currentInstruction = 0; currentInstruction < instructions.size(); ++currentInstruction) {
            instructions[currentInstruction]->execute(context);
        }
    }
}