#include <iostream>
#include <string>
#include <regex>
#include <cstdlib>
#include "process_list.h"
#include "console.h"
#include "scheduler.h"
#include "PrintCommand.h"
#include "DeclareCommand.h"
#include "AddCommand.h"
#include "SubtractCommand.h"
#include "SleepCommand.h"
#include "ForCommand.h"
#include "ReadCommand.h"
#include "WriteCommand.h"
#include "Config.h"
#include <fstream>
#include <sstream>
#include <vector>
#include <iomanip>


using std::cin;
using std::cout;
using std::endl;
using std::regex;
using std::smatch;
using std::string;

ProcessList processes;

// Function to parse a single instruction
std::shared_ptr<Command> parseInstruction(const std::string& instruction) {
    std::istringstream iss(instruction);
    std::string cmd;
    iss >> cmd;
    
    if (cmd == "DECLARE") {
        std::string varName;
        uint16_t value;
        iss >> varName >> value;
        return std::make_shared<DeclareCommand>(varName, value);
    }
    else if (cmd == "ADD") {
        std::string var1, var2, var3;
        iss >> var1 >> var2 >> var3;
        return std::make_shared<AddCommand>(var1, var2, var3);
    }
    else if (cmd == "SUBTRACT") {
        std::string var1, var2, var3;
        iss >> var1 >> var2 >> var3;
        return std::make_shared<SubtractCommand>(var1, var2, var3);
    }
    else if (cmd.substr(0, 5) == "PRINT") {
        std::string rest = instruction.substr(5); // everything after 'PRINT'
        // Trim whitespace
        rest.erase(0, rest.find_first_not_of(" \t("));
        rest.erase(rest.find_last_not_of(" \t)") + 1);

        // Remove surrounding quotes if present
        if (rest.length() >= 2 && rest[0] == '"' && rest[rest.length()-1] == '"') {
            rest = rest.substr(1, rest.length()-2);
        }
        return std::make_shared<PrintCommand>(rest);
    }
    else if (cmd == "SLEEP") {
        uint8_t sleepTime;
        iss >> sleepTime;
        return std::make_shared<SleepCommand>(sleepTime);
    }
    else if (cmd == "READ") {
        std::string varName;
        std::string addressStr;
        iss >> varName >> addressStr;
        // Parse hex address
        uint16_t address = std::stoul(addressStr, nullptr, 16);
        return std::make_shared<ReadCommand>(varName, address);
    }
    else if (cmd == "WRITE") {
        std::string addressStr, varName;
        iss >> addressStr >> varName;
        // Parse hex address
        uint16_t address = std::stoul(addressStr, nullptr, 16);
        return std::make_shared<WriteCommand>(address, varName);
    }
    else {
        throw std::runtime_error("Unknown instruction: " + cmd);
    }
}

// Function to parse instruction string and create commands
std::vector<std::shared_ptr<Command>> parseInstructions(const std::string& instructionString) {
    std::vector<std::shared_ptr<Command>> commands;
    std::istringstream iss(instructionString);
    std::string instruction;
    
    while (std::getline(iss, instruction, ';')) {
        // Trim whitespace
        instruction.erase(0, instruction.find_first_not_of(" \t"));
        instruction.erase(instruction.find_last_not_of(" \t") + 1);
        
        if (!instruction.empty()) {
            try {
                commands.push_back(parseInstruction(instruction));
            } catch (const std::exception& e) {
                throw std::runtime_error("Error parsing instruction '" + instruction + "': " + e.what());
            }
        }
    }
    
    return commands;
}

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
void createOrResumeScreen(const string &cmd, const string &name, int memSize, Scheduler& scheduler)
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
            int pid = processes.findProcessByName(name);

            if (pid == -1) {
                cout << "Failed to create process." << endl;
                return;
            }

            if (!scheduler.getMemoryManager().allocate(pid, memSize)) {
                cout << "Error: Not enough memory to allocate " << memSize << " bytes for process '" << name << "'." << endl;
                processes.removeProcess(pid);
                return;
            }

            processes.withProcessByRef(pid, [&](process& proc) {
                proc.setMemorySize(memSize);
                proc.setMemoryManager(&scheduler.getMemoryManager());
                });

            scheduler.addProcess(processes.findProcess(pid));
            cout << "Process '" << name << "' created and added to scheduler with " << memSize << " bytes of memory." << endl;
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

// Create process with custom instructions
// Update function signature to accept Scheduler&
void createProcessWithInstructions(const string &processName, int memorySize, const string &instructions, Scheduler& scheduler)
{
    if (processes.ifProcessNameExists(processName))
    {
        cout << "Process '" << processName << "' already exists." << endl;
        return;
    }
    
    try {
        // Parse instructions
        std::vector<std::shared_ptr<Command>> commands = parseInstructions(instructions);
        
        // Validate instruction count (1-50)
        if (commands.size() < 1 || commands.size() > 50) {
            cout << "Invalid command: Instruction count must be between 1 and 50." << endl;
            return;
        }
        
        // Create process
        processes.addNewProcess(-1, 0, processName);
        int pid = processes.findProcessByName(processName);
        
        if (pid == -1) {
            cout << "Failed to create process." << endl;
            return;
        }

        if (!scheduler.getMemoryManager().allocate(pid, memorySize)) {
            cout << "Error: Not enough memory to allocate " << memorySize << " bytes for process '" 
                 << processName << "'." << endl;
            processes.removeProcess(pid);
            return;
        }
        
        // Add instructions to process
        processes.withProcessByRef(pid, [&](process& proc) {
            proc.clearInstructions();
            proc.setMemoryManager(&scheduler.getMemoryManager()); 
            proc.setMemorySize(memorySize);
            for (const auto& cmd : commands) {
                proc.addInstruction(cmd);
            }
        });
        
        // Add process to scheduler's ready queue
        scheduler.addProcess(processes.findProcess(pid));
        
        cout << "Process '" << processName << "' created with " << commands.size() << " instructions." << endl;
        
    } catch (const std::exception& e) {
        cout << "Error creating process: " << e.what() << endl;
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
    regex pattern(R"(^screen -([rs])(?:\s+([^\s]+))(?:\s+(\d+))?\s*$)");
    regex customPattern("^screen -c\\s+(\\S+)\\s+(\\d+)\\s+\"(.*)\"\\s*$");

    smatch match;
    MemoryManager memoryManager(config.getMaxOverallMem(), config.getMemPerFrame());
    Scheduler scheduler(processes, config, memoryManager);

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

        trimSpaces(command);

        //cout << "[DEBUG] Raw command: " << command << endl;

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
        // Handle screen -c command for custom instructions
        else if (std::regex_match(command, match, customPattern))
        {
            string processName = match[1];
            int memorySize = std::stoi(match[2]);
            string instructions = match[3];
            
            createProcessWithInstructions(processName, memorySize, instructions, scheduler);
        }
        // create new process or resume existing screen session
        else if (std::regex_match(command, match, pattern))
        {

            if (regex_match(command, match, pattern))
            {
                string mode = match[1];  // 's' or 'r'
                string name = match[2];  // process name

                trimSpaces(name);

                if (name.find(' ') != string::npos)
                {
                    cout << "Screen name cannot contain spaces. Usage: screen -s <name> <memory_size> or screen -r <name>" << endl;
                }
                else if (name.empty())
                {
                    cout << "Please provide a screen name. Usage: screen -s <name> <memory_size> or screen -r <name>" << endl;
                }
                else if (mode == "r")
                {
                    createOrResumeScreen("screen -r", name, -1, scheduler);
                }
                else if (mode == "s")
                {
                    if (match.size() < 4 || match[3].str().empty())
                    {
                        cout << "Memory size is required. Usage: screen -s <name> <memory_size>" << endl;
                    }
                    else
                    {
                        int memSize = stoi(match[3]);

                        auto isPowerOfTwo = [](int n)
                            {
                                return n >= 64 && n <= 65536 && (n & (n - 1)) == 0;
                            };

                        if (!isPowerOfTwo(memSize))
                        {
                            cout << "Invalid memory allocation. Size must be a power of 2 between 64 and 65536." << endl;
                        }
                        else
                        {
                            createOrResumeScreen("screen -s", name, memSize, scheduler);
                        }
                    }
                }
            }
        }
        else if (command == "screen --help")
        {
            cout << "Available screen commands:" << endl;
            cout << "  screen -s <name>     : Create new process" << endl;
            cout << "  screen -r <name>     : Resume existing process" << endl;
            cout << "  screen -c <name> <mem> \"<instructions>\" : Create process with custom instructions" << endl;
            cout << "  screen -ls           : List all processes" << endl;
        }
        else
        {
            cout << "Unknown command. Please try again." << endl;
        }
    }
}