#include <iostream>
#include <string>
#include "Config.h"
#include "OsEmulator.h"
#include <fstream>
#include <filesystem>

using namespace std;
int main()
{
    cout << "Program Start\n" << endl;

    string command;
    Config config("Config/config.txt");

    while (true)
    {
        cout << "Enter command: ";
        getline(cin, command);

        trimSpaces(command);

        if (command == "initialize")
        {
            startEmulator(config);
            break;
        }
        else if (command == "exit")
        {
            cout << "exit command recognized. Exiting program." << endl;
            break;
        }
        else
        {
            cout << "Unknown command. Please try again." << endl;
        }
    }

    return 0;
}