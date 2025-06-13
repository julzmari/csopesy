#include <iostream>
#include <string>
#include <regex>
#include <cstdlib>
#include "process_list.h"
#include "console.h"

using namespace std;
ProcessList processes;

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void trimSpaces(string& str) {
    str.erase(0, str.find_first_not_of(" \t"));
    str.erase(str.find_last_not_of(" \t") + 1);
}

void printHeader() {
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
void createOrResumeScreen(const string& cmd, const string& name) {
    if (cmd == "screen -s") { // Create new process
        if (processes.ifProcessNameExists(name)) {
            cout << "Screen '" << name << "' already exists. Use 'screen -r " << name << "' to resume." << endl;
        }
        else {
            std::vector<std::string> instructions;
			processes.addNewProcess(-1, 0, name, instructions);
			processes.printAllProcesses();
        }
    }
    else if (cmd == "screen -r") { // Resume existing process
        if (processes.ifProcessNameExists(name)) {
            int pid = processes.findProcessByName(name);
            process curr_proc = processes.findProcess(pid);
            console proc_console(curr_proc);
            proc_console.handleScreen();
        }
        else {
            cout << "No screen found with name '" << name << "'. Use 'screen -s " << name << "' to create one." << endl;
        }
    }
}

void startEmulator() {
    string command;
    regex pattern(R"(^screen -[rs](?:\s+[^\s]+(?:\s+[^\s]+)*)?\s*$)");
    smatch match;

    clearScreen();
    printHeader();

    while (true) {
        cout << "\nEnter command: ";
        getline(cin, command);

        // Trim leading and trailing spaces
        trimSpaces(command);

        if (command == "initialize" ||
            command == "screen" ||
            command == "scheduler-test" ||
            command == "scheduler-stop" ||
            command == "report-util") {
            cout << command << " command recognized. Doing something." << endl;
        }
        else if (command == "clear") {
            clearScreen();
            printHeader();
        }
        else if (command == "exit") {
            cout << "Exit command recognized. Exiting program." << endl;
            break;
        }
        // create new process or resume existing screen session
        else if (regex_match(command, match, pattern)) {
            string prefix = command.substr(0, 9); // "screen -s" or "screen -r"
            string name = command.substr(9);

            // Trim leading and trailing spaces
            trimSpaces(name);

            if (name.find(' ') != string::npos) {
                cout << "Screen name cannot contain spaces. Usage: screen -s <name> or screen -r <name>" << endl;
            }
            else if (!name.empty()) {
                createOrResumeScreen(prefix, name);
            }
            else {
                cout << "Please provide a screen name. Usage: screen -s <name> or screen -r <name>" << endl;
            }
        }
        else if (command == "screen --help") {

        }
        else {
            cout << "Unknown command. Please try again." << endl;
        }
    }
}
