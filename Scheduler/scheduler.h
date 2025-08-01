#pragma once
#ifdef _WIN32
#include <windows.h>
#endif
#include <myProcess.h>
#include <thread>
#include "process_list.h"
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "Config.h"
#include "MemoryManager.h"

class Scheduler
{
public:
    Scheduler(ProcessList &plist, Config &config, MemoryManager &memManager);
    void start();
    void stop();
    void addProcess(const process &proc);
    void startBatchGeneration();
    void stopBatchGeneration();
    int getNumCores() const { return numCores; }
    int getCoreAssignment(int core) const { return coreAssignments[core]; }
    MemoryManager& getMemoryManager() { return memoryManager; }


private:
    int batchFreq;
    int minIns, maxIns;
    int delaysPerExec;
    int quantum;
    int numCores;
    std::atomic<int> processCounter{0};
    int quantumCycle = 0; 
    int minMemPerProc, maxMemPerProc;
    void snapshotMemory(int cycle);

    void schedulerThreadFunc();
    void workerThreadFunc(int coreId);

    SchedulerAlgorithm schedulerType;
    ProcessList &processList;
    MemoryManager &memoryManager;
    std::vector<std::thread> workers;
    std::thread schedulerThread;
    std::queue<int> readyQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> running;
    std::atomic<bool> batchGenerating = false;
    std::thread batchGeneratorThread;
    std::shared_ptr<Command> generateForBlock(int currentDepth, const std::string &procName);
    std::vector<int> coreAssignments; // coreAssignments[coreId] = pid or -1 if idle
};