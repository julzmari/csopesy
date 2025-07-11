#include <iostream>
#include <string>
#include <regex>
#include <cstdlib>
#include "process_list.h"
#include "console.h"
#include "scheduler.h"
#include "PrintCommand.h"
#include "Config.h"
#include <fstream>

using std::cin;
using std::cout;
using std::endl;
using std::regex;
using std::smatch;
using std::string;

ProcessList processes;

void clearScreen()
{
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void trimSpaces(string &str)
{
    str.erase(0, str.find_first_not_of(" \t"));
    str.erase(str.find_last_not_of(" \t") + 1);
}

void printHeader()
{
    const string RED = "\033[1;31m";
    const string GREEN = "\033[1;32m";
    const string YELLOW = "\033[1;33m";
    const string BLUE = "\033[1;34m";
    const string RESET = "\033[0m";

    cout << BLUE << R"(
     __         __  __     __    __     ______     ______   
    /\ \       /\ \/\ \   /\ "-./  \   /\  __ \   /\  ___\  
    \ \ \____  \ \ \_\ \  \ \ \-./\ \  \ \ \/\ \  \ \___  \ 
     \ \_____\  \ \_____\  \ \_\ \ \_\  \ \_____\  \/\_____\
      \/_____/   \/_____/   \/_/  \/_/   \/_____/   \/_____/

    )" << endl;

    cout << YELLOW << "Welcome to LUMOS Commandline!" << endl;
    cout << GREEN << "Type " << RED << "exit" << GREEN << " to exit anytime, " << RED << "clear" << GREEN << " to refresh your screen." << endl;
    cout << RESET;
}

// Create or resume screen
void createOrResumeScreen(const string &cmd, const string &name)
{
    if (cmd == "screen -s")
    { // Create new process
        if (processes.ifProcessNameExists(name))
        {
            cout << "Screen '" << name << "' already exists. Use 'screen -r " << name << "' to resume." << endl;
        }
        else
        {
            std::vector<std::string> instructions;
            processes.addNewProcess(-1, 0, name);
            processes.printAllProcesses();
        }
    }
    else if (cmd == "screen -r")
    { // Resume existing process
        if (processes.ifProcessNameExists(name))
        {
            int pid = processes.findProcessByName(name);
            process curr_proc = processes.findProcess(pid);
            console proc_console(processes, curr_proc);
            proc_console.handleScreen();
            printHeader();
        }
        else
        {
            cout << "No screen found with name '" << name << "'. Use 'screen -s " << name << "' to create one." << endl;
        }
    }
}

void listScreens()
{
    //std::cout << "[DIAG] listScreens called" << std::endl;
    processes.printAllProcesses();
}

void generateReport(const Config &config, bool toConsole = true)
{
    int totalCores = config.getNumCPU();
    int usedCores = 0;

    std::ostringstream output;

    // Header
    output << "CPU utilization: ";
    int runningCount = 0;

    for (const auto &pair : processes.getAll())
    {
        const process &proc = pair.second;
        if (proc.getState() == ProcessState::RUNNING)
            runningCount++;
    }

    usedCores = runningCount;
    int utilization = (100 * usedCores) / totalCores;
    output << utilization << "%\n";
    output << "Cores used: " << usedCores << "\n";
    output << "Cores available: " << (totalCores - usedCores) << "\n";
    output << "------------------------------------\n";

    output << "Running processes:\n";
    for (const auto &pair : processes.getAll())
    {
        const process &proc = pair.second;
        if (proc.getState() == ProcessState::RUNNING)
        {
            output << std::left << std::setw(15)
                   << proc.getProcessName()
                   << std::setw(25) << proc.getCreationTime()
                   << "Core: " << proc.getCoreId()
                   << " " << proc.getCurrentLine()
                   << " / " << proc.getLineCount() << "\n";
        }
    }

    output << "\nFinished processes:\n";
    for (const auto &pair : processes.getAll())
    {
        const process &proc = pair.second;
        if (proc.getState() == ProcessState::FINISHED)
        {
            output << std::left << std::setw(15)
                   << proc.getProcessName()
                   << std::setw(25) << proc.getCreationTime()
                   << "Finished "
                   << proc.getLineCount() << " / " << proc.getLineCount() << "\n";
        }
    }

    if (toConsole)
    {
        std::cout << output.str();
    }
    else
    {
        std::ofstream file("csopesy-log.txt");
        file << output.str();
        file.close();
        std::cout << "Report generated at C:/csopesy-log.txt!\n";
    }

    // Diagnostic: print all PIDs and their states at report time
    //std::cout << "[DIAG] generateReport: process states at report time:" << std::endl;
    //for (const auto &pair : processes.getAll())
    //{
     //   const process &proc = pair.second;
    //    std::cout << "PID: " << pair.first << " State: " << static_cast<int>(proc.getState()) << std::endl;
    //}
}

void startEmulator(Config &config)
{
    string command;
    regex pattern(R"(^screen -[rs](?:\s+[^\s]+(?:\s+[^\s]+)*)?\s*$)");
    smatch match;
    MemoryManager memoryManager(config.getMaxOverallMem(), config.getMemPerFrame());
    Scheduler scheduler(processes, config, memoryManager);

    // PRINT COMMMANDS
    /*for (int i = 1; i <= 10; ++i)
    {
        std::vector<std::string> instructions;
        for (int j = 1; j <= 100; ++j)
        {
            instructions.push_back("Print command " + std::to_string(j));
        }

        processes.addNewProcess(-1, 0, "Process" + std::to_string(i));
        int pid = processes.findProcessByName("Process" + std::to_string(i));
        scheduler.addProcess(processes.findProcess(pid));
    }*/

    scheduler.start();

    clearScreen();
    printHeader();

    cout << "Scheduler initialized with " << config.getNumCPU() << " cores and "
         << (config.getSchedulerAlgorithm() == SchedulerAlgorithm::FCFS ? "FCFS" : "Round Robin")
         << " algorithm." << endl;

    while (true)
    {
        cout << "\nEnter command: ";
        getline(cin, command);

        // Trim leading and trailing spaces
        trimSpaces(command);

        if (command == "scheduler-start")
        {
            scheduler.startBatchGeneration();
        }
        else if (command == "scheduler-stop")
        {
            scheduler.stopBatchGeneration();
        }
        else if (command == "screen -ls")
        {
            // listScreens();
            generateReport(config, true); // console
        }
        else if (command == "report-util")
        {
            generateReport(config, false); // file
        }
        else if (command == "clear")
        {
            clearScreen();
            printHeader();
        }
        else if (command == "exit")
        {
            cout << "Exit command recognized. Exiting program." << endl;
            scheduler.stop();
            break;
        }
        // create new process or resume existing screen session
        else if (regex_match(command, match, pattern))
        {
            string prefix = command.substr(0, 9); // "screen -s" or "screen -r"
            string name = command.substr(9);

            // Trim leading and trailing spaces
            trimSpaces(name);

            if (name.find(' ') != string::npos)
            {
                cout << "Screen name cannot contain spaces. Usage: screen -s <name> or screen -r <name>" << endl;
            }
            else if (!name.empty())
            {
                createOrResumeScreen(prefix, name);
            }
            else
            {
                cout << "Please provide a screen name. Usage: screen -s <name> or screen -r <name>" << endl;
            }
        }
        else if (command == "screen --help")
        {
        }
        else
        {
            cout << "Unknown command. Please try again." << endl;
        }
    }
}
