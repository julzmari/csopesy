#pragma once
#include "Process/process_list.h"
#include <thread>
#include <vector>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

class Scheduler
{
public:
    Scheduler(ProcessList &plist, int numCores);
    void start();
    void stop();
    void addProcess(const process &proc);

private:
    void schedulerThreadFunc();
    void workerThreadFunc(int coreId);

    ProcessList &processList;
    int numCores;
    std::vector<std::thread> workers;
    std::thread schedulerThread;
    std::queue<process> readyQueue;
    std::mutex queueMutex;
    std::condition_variable cv;
    std::atomic<bool> running;
};