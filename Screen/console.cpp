#include "console.h"
#include "myProcess.h"
#include "process_list.h"
#include <fstream>
#include <string>

class process;

#ifdef _WIN32
#include <windows.h>
#endif

console::console(ProcessList &plist, const process &proc) : processList(plist), proc(proc), totalLines(100) {}

// Simulate inside-screen interaction
void console::handleScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif

    std::cout << "=== Screen: " << proc.getProcessName() << " ===" << std::endl;
    std::cout << "Process: " << proc.getProcessName() << std::endl;
    std::cout << "Instruction: Line " << proc.getCurrentLine() << " / " << proc.getLineCount() << std::endl;
    std::cout << "Created at: " << proc.getCreationTime() << std::endl;

    std::string input;
    while (true)
    {
        std::cout << "\nroot:/>";
        std::getline(std::cin, input);

        if (input == "exit")
        {
#ifdef _WIN32
            system("cls");
#else
            system("clear");
#endif
            break;
        }
        else if (input == "process-smi")
        {
            process latestProc = processList.findProcess(proc.getPid());
            std::cout
                << "Process: " << latestProc.getProcessName() << "\n"
                << "ID: " << latestProc.getPid() << "\n"
                << "Current instruction line: " << latestProc.getCurrentLine() << "\n"
                << "Lines of code: " << latestProc.getLineCount() << "\n"
                << "Created at: " << latestProc.getCreationTime() << "\n";

            const auto &logs = latestProc.getLogs();

            if (!logs.empty())
            {
                std::cout << "\n--- Print Logs ---\n";
                for (const auto &entry : logs)
                {
                    std::cout << entry;
                }
            }

            else
            {
                std::cout << "\nNo print logs found.\n";
            }

            if (latestProc.getState() == ProcessState::FINISHED)
            {
                std::cout << "\nFinished!" << std::endl;
            }
        }
        else
        {
            std::cout << "Unknown screen command. Only 'exit' is supported." << std::endl;
        }
    }
}