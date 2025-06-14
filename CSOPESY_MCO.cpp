#include <iostream>
#include <string>
#include "Config.h"
#include "OsEmulator.h"

using namespace std;
int main() {

    string command;
    Config config("Config/config.txt");

    while (true) {
        cout << "Enter command: ";
        getline(cin, command);

        // Trim leading and trailing spaces
        trimSpaces(command);

        if (command == "initialize") {
            startEmulator();
            break;
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