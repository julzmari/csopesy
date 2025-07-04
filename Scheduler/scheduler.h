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
#include "config.h"

class Scheduler
{
public:
    Scheduler(ProcessList &plist, Config &config);
    void start();
    void stop();
    void addProcess(const process &proc);
    void startBatchGeneration();
    void stopBatchGeneration();

private:
    int batchFreq;
    int minIns, maxIns;
    int delaysPerExec;
    int quantum;
    int numCores;
    int processCounter = 0;

    void schedulerThreadFunc();
    void workerThreadFunc(int coreId);

    SchedulerAlgorithm schedulerType;
    ProcessList &processList;
    std::vector<std::thread> workers;
    std::thread schedulerThread;
    std::queue<int> readyQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> running;
    std::atomic<bool> batchGenerating = false;
    std::thread batchGeneratorThread;
    std::shared_ptr<Command> generateForBlock(int currentDepth, const std::string &procName);
};