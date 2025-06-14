#include "console.h"
#include "process.h"

class process;

#ifdef _WIN32
#include <windows.h>
#endif

console::console() : proc(), totalLines(100) {}

console::console(const process& proc) : proc(proc), totalLines(100) {}

// Simulate inside-screen interaction
void console::handleScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    std::cout << "=== Screen: " << proc.getProcessName() << " ===" << std::endl;
    std::cout << "Process: " << proc.getProcessName() << std::endl;
    std::cout << "Instruction: Line " << proc.getCurrentLine() << " / " << totalLines << std::endl;
    std::cout << "Created at: " << proc.getCreationTime() << std::endl;

    std::string input;
    while (true) {
        std::cout << "\nroot:/>";
        std::getline(std::cin, input);

        if (input == "exit") {
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif
            break;
        }
        else if (input == "process-smi") {
            std::cout
                << "Process: " << proc.getProcessName() << "\n"
                << "ID: " << proc.getPid() << "\n\n"
                << "Current instruction line: " << proc.getCurrentLine() << "\n"
                << "Lines of code: " << totalLines;
        }
        else {
            std::cout << "Unknown screen command. Only 'exit' is supported." << std::endl;
        }
    }
}