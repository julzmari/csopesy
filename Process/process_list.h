#pragma once
#include "myProcess.h"
#include <unordered_map>

class ProcessList
{
private:
    std::unordered_map<int, process> processMap;
    std::unordered_map<std::string, int> nameToPidMap;
    int lastPid = 0;

public:
    process findProcess(int pid);
    void addNewProcess(int coreId, int priority, const std::string &processName);
    void updateProcess(const process &proc);
    void printAllProcesses();
    int findProcessByName(const std::string &processName);
    bool ifProcessNameExists(const std::string &processName);
};
