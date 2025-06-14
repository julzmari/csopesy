#include <windows.h>
#include <process.h>
#include <thread>
#include "Scheduler.h"
#include "PrintCommand.h"

Scheduler::Scheduler(ProcessList &plist, int numCores)
    : processList(plist), numCores(numCores), running(false) {}

void Scheduler::start()
{
    running = true;
    for (int i = 0; i < numCores; ++i)
        workers.emplace_back(&Scheduler::workerThreadFunc, this, i);
    schedulerThread = std::thread(&Scheduler::schedulerThreadFunc, this);
}

void Scheduler::stop()
{
    running = false;
    cv.notify_all();
    for (auto &t : workers)
        if (t.joinable())
            t.join();
    if (schedulerThread.joinable())
        schedulerThread.join();
}

void Scheduler::addProcess(const process &proc)
{
    process procCopy = proc;
    procCopy.setState(ProcessState::READY);
    std::lock_guard<std::mutex> lock(queueMutex);
    readyQueue.push(procCopy);
    cv.notify_one();
}

void Scheduler::schedulerThreadFunc()
{
    while (running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        cv.notify_all();
    }
}

void Scheduler::workerThreadFunc(int coreId)
{
    while (running)
    {
        process proc;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this]
                    { return !readyQueue.empty() || !running; });
            if (!running)
                break;
            proc = readyQueue.front();
            readyQueue.pop();
            proc.setState(ProcessState::RUNNING);
            proc.setCoreId(coreId);
            processList.updateProcess(proc);
        }
        for (int i = 0; i < proc.getLineCount(); ++i)
        {
            proc.executeNextInstruction(coreId);
            proc.setCurrentLine(i + 1);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }

        proc.setState(ProcessState::TERMINATED);
        processList.updateProcess(proc);
    }
}