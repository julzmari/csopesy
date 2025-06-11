#include "Config.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdexcept>
#include <iostream>
#include <iomanip>
#include <filesystem>

namespace fs = filesystem;
using namespace std;

Config::Config(const string& filename) :
    numCPU(0),
    schedulerAlgorithm(SchedulerAlgorithm::FCFS),
    quantumCycles(0),
    batchProcessFreq(0),
    minIns(0),
    maxIns(0),
    delaysPerExec(0)
{

    vector<fs::path> possiblePaths = {
        filename,                          
        fs::path("..") / filename,         
        fs::path("../..") / filename,      
        fs::path("../../..") / filename
    };

    ifstream file;
    for (const auto& path : possiblePaths) {
        file.open(path);
        if (file.is_open()) {
            break;
        }
    }

    if (!file.is_open()) {
        cerr << "Warning: Could not open config file '" << filename
            << "' in any standard location. Using default values." << endl;
        return;
    }

    string line;

    while (getline(file, line)) {
        istringstream iss(line);
        string key;
        if (iss >> key) {
            if (key == "num-cpu") {
                iss >> numCPU;
                if (numCPU < 1 || numCPU > 128) {
                    throw out_of_range("num-cpu must be between 1 and 128");
                }
            }
            else if (key == "scheduler") {
                string algo;
                iss >> algo;
                // Remove quotes if present
                algo.erase(remove(algo.begin(), algo.end(), '"'), algo.end());

                if (algo == "fcfs") {
                    schedulerAlgorithm = SchedulerAlgorithm::FCFS;
                }
                else if (algo == "rr") {
                    schedulerAlgorithm = SchedulerAlgorithm::RR;
                }
                else {
                    throw invalid_argument("Invalid scheduler algorithm");
                }
            }
            else if (key == "quantum-cycles") {
                iss >> quantumCycles;
                if (quantumCycles < 1 || quantumCycles > 4294967296) {
                    throw out_of_range("quantum-cycles must be between 1 and 4294967296");
                }
            }
            else if (key == "batch-process-freq") {
                iss >> batchProcessFreq;
                if (batchProcessFreq < 1 || batchProcessFreq > 4294967296) {
                    throw out_of_range("batch-process-freq must be between 1 and 4294967296");
                }
            }
            else if (key == "min-ins") {
                iss >> minIns;
                if (minIns < 1 || minIns > 4294967296) {
                    throw out_of_range("min-ins must be between 1 and 4294967296");
                }
            }
            else if (key == "max-ins") {
                iss >> maxIns;
                if (maxIns < minIns) {
                    throw out_of_range("max-ins must be >= min-ins");
                } else if (maxIns < 1 || maxIns > 4294967296) {
                    throw out_of_range("max-ins must be between 1 and 4294967296");
				}
            }
            else if (key == "delay-per-exec") {
                iss >> delaysPerExec;
                if (delaysPerExec < 0) {
                    throw out_of_range("delay-per-exec must be between 0 and 4294967296");
                }
            }
        }
    }
    file.close();
}

void Config::printConfig() const {
    const int colWidth = 25;

    cout << "\n=== Current Configuration ===\n";
    cout << left << setw(colWidth) << "Number of CPUs:" << numCPU << "\n";

    cout << setw(colWidth) << "Scheduler Algorithm:"
        << (schedulerAlgorithm == SchedulerAlgorithm::FCFS ? "FCFS" : "Round Robin") << "\n";

    if (schedulerAlgorithm == SchedulerAlgorithm::RR) {
        cout << setw(colWidth) << "Quantum Cycles:" << quantumCycles << "\n";
    }

    cout << setw(colWidth) << "Batch Process Frequency:" << batchProcessFreq << " cycles\n";
    cout << setw(colWidth) << "Min Instructions/Process:" << minIns << "\n";
    cout << setw(colWidth) << "Max Instructions/Process:" << maxIns << "\n";
    cout << setw(colWidth) << "Delay per Execution:" << delaysPerExec << " cycles\n";
    cout << "===========================\n";
}