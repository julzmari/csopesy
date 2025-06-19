#include <iostream>
#include <string>
#include "Config.h"
#include "OsEmulator.h"

using namespace std;
int main() {

    string command;
    Config config("Config/config.txt");

    while (true) {
        cout << "root:/> ";
        getline(cin, command);

        // Trim leading and trailing spaces
        trimSpaces(command);

        if (command == "initialize") {
            startEmulator(config);
            break;
        }
        else if (command == "exit") {
            cout << "exit command recognized. Exiting program." << endl;
            break;
        }
    }

	return 0;
}