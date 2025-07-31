#pragma once
#include "myProcess.h"
#include <unordered_map>
#include <mutex> // Added for mutex

class ProcessList
{
private:
    std::unordered_map<int, process> processMap;
    std::unordered_map<std::string, int> nameToPidMap;
    int lastPid = 0;
    mutable std::mutex mtx; // Mutex for thread safety

public:
    process findProcess(int pid);
    process &findProcessByRef(int pid);
    void addNewProcess(int coreId, int priority, const std::string &processName);
    void updateProcess(const process &proc);
    void printAllProcesses();
    int findProcessByName(const std::string &processName);
    bool ifProcessNameExists(const std::string &processName);
    int getNextAvailablePid();
    void removeProcess(int pid);


    // Safe access to a process by reference using a lambda
    template<typename Func>
    void withProcessByRef(int pid, Func fn) {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = processMap.find(pid);
        if (it != processMap.end()) {
            fn(it->second);
        }
    }

    const std::unordered_map<int, process> &getAll() const
    {
        std::lock_guard<std::mutex> lock(mtx);
        return processMap;
    }
};
