#ifdef _WIN32
#include <windows.h>
#endif
#ifdef _WIN32
#include <process.h>
#endif
#include <thread>
#include "scheduler.h"
#include "PrintCommand.h"
#include "config.h"

Scheduler::Scheduler(ProcessList &plist, Config& config)
    : processList(plist), numCores(config.getNumCPU()), running(false),
    batchFreq(config.getBatchProcessFreq()),
    minIns(config.getMinIns()), maxIns(config.getMaxIns()),
    delaysPerExec(config.getDelaysPerExec()) {
}

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
            auto instruction = proc.getCurrentInstruction();
            if (instruction) {
                instruction->execute(proc);
            }
            proc.setCurrentLine(i + 1); // Move to the next instruction
            processList.updateProcess(proc);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        proc.setState(ProcessState::FINISHED);
        processList.updateProcess(proc);
    }
}

void Scheduler::startBatchGeneration() {
    if (batchGenerating) return;
    batchGenerating = true;

    batchGeneratorThread = std::thread([this]() {

        int processCounter = 0;

        while (batchGenerating) {

            int insCount = minIns + rand() % (maxIns - minIns + 1);

            std::vector<std::shared_ptr<Command>> cmds;

            std::string procName = "Process" + std::to_string(++processCounter);
            processList.addNewProcess(-1, 0, procName);
            int pid = processList.findProcessByName(procName);
            process proc = processList.findProcess(pid);

            for (int i = 0; i < insCount; ++i) {
                proc.addInstruction(std::make_shared<PrintCommand>("Command " + std::to_string(i + 1), delaysPerExec));
            }

            addProcess(proc);
            std::this_thread::sleep_for(std::chrono::milliseconds(batchFreq));
        }
        });
}

void Scheduler::stopBatchGeneration() {
    batchGenerating = false;
    if (batchGeneratorThread.joinable()) {
        batchGeneratorThread.join();
    }
}