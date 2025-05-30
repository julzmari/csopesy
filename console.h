#include <iostream>
#include <string>

using namespace std;

class console {
private:
    std::string name;
    int currentLine = 1;
    std::string timestamp;
    int totalLines = 100;

public:
    // Constructor
    console() : name(""), timestamp("") {} // Default constructor

    // New console
    console(const std::string& name, const std::string& timestamp)
        : name(name), timestamp(timestamp) {
    }

    // Simulate inside-screen interaction
    void handleScreen() {
#ifdef _WIN32
        system("cls");
#else
        system("clear");
#endif

        cout << "=== Screen: " << name << " ===" << endl;
        cout << "Process: " << name << endl;
        cout << "Instruction: Line " << currentLine << " / " << totalLines << endl;
        cout << "Created at: " << timestamp << endl;

        string input;
        while (true) {
            cout << "\n(" << name << ") Type 'exit' to return to main menu: ";
            getline(cin, input);

            if (input == "exit") {
#ifdef _WIN32
                system("cls");
#else
                system("clear");
#endif

                break;
            }
            else {
                cout << "Unknown screen command. Only 'exit' is supported." << endl;
            }
        }
    }
};