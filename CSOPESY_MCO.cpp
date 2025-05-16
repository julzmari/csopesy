#include <iostream>
#include <string>
#include <cstdlib> 
using namespace std;

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

void handleCommandPrint(const string& cmd) {
    cout << cmd << " command recognized. Doing something." << endl;
}

int main() {
    string command;

    printHeader();

    while (true) {
        cout << "\nEnter command: ";
        getline(cin, command);

        if (command == "initialize" ||
            command == "screen" ||
            command == "scheduler-test" ||
            command == "scheduler-stop" ||
            command == "report-util") {
            handleCommandPrint(command);
        }
        else if (command == "clear") {
            system("cls");
            printHeader();
        }
        else if (command == "exit") {
            cout << "exit command recognized. Exiting program." << endl;
            break;
        }
        else {
            cout << "Unknown command. Please try again." << endl;
        }
    }
    
    return 0;
}
