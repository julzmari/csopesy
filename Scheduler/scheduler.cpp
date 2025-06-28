#include <thread>
#include "scheduler.h"
#include "PrintCommand.h"
#include "AddCommand.h"
#include "SubtractCommand.h"
#include "DeclareCommand.h"
#include "SleepCommand.h"
#include "ForCommand.h"
#include "config.h"
#include <iostream>
using namespace std;

Scheduler::Scheduler(ProcessList &plist, Config &config)
    : processList(plist),
      numCores(config.getNumCPU()),
      running(false),
      batchFreq(config.getBatchProcessFreq()),
      minIns(config.getMinIns()),
      maxIns(config.getMaxIns()),
      delaysPerExec(config.getDelaysPerExec()),
      schedulerType(config.getSchedulerAlgorithm()),
      quantum(config.getQuantumCycles())
{
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
    std::lock_guard<std::mutex> lock(queueMutex);
    readyQueue.push(proc.getPid());
    cv.notify_one();
}

void Scheduler::schedulerThreadFunc()
{
    while (running)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(delaysPerExec));
        cv.notify_all();
    }
}

std::shared_ptr<Command> Scheduler::generateForBlock(int currentDepth, const std::string &procName)
{
    std::vector<std::shared_ptr<Command>> nestedInstructions;
    int repeatCount = 1 + rand() % 3; // 1 to 3 iterations

    nestedInstructions.push_back(std::make_shared<PrintCommand>());

    if (currentDepth < 3)
    {
        if (rand() % 2 == 0)
        {
            nestedInstructions.push_back(generateForBlock(currentDepth + 1, procName));
        }
    }

    return std::make_shared<ForCommand>(nestedInstructions, repeatCount);
}

void Scheduler::workerThreadFunc(int coreId)
{
    while (running)
    {
        int pid;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this]
                    { return !readyQueue.empty() || !running; });
            if (!running)
                break;
            pid = readyQueue.front();
            readyQueue.pop();
        }

        process &proc = processList.findProcessByRef(pid);
        proc.setState(ProcessState::RUNNING);
        proc.setCoreId(coreId);
        processList.updateProcess(proc);

        // FCFS Implementation
        if (schedulerType == SchedulerAlgorithm::FCFS)
        {
            for (int i = proc.getCurrentLine(); i < proc.getLineCount(); ++i)
            {
                auto instruction = proc.getCurrentInstruction();
                if (instruction)
                {
                    instruction->execute(proc);
                }
                proc.setCurrentLine(i + 1);
                processList.updateProcess(proc);
                std::this_thread::sleep_for(std::chrono::milliseconds(delaysPerExec));
            }
            proc.setState(ProcessState::FINISHED);
            processList.updateProcess(proc);
        }

        // Round Robin Implementation
        else if (schedulerType == SchedulerAlgorithm::RR)
        {
            int linesRun = 0;
            for (; linesRun < quantum && proc.getCurrentLine() < proc.getLineCount(); ++linesRun)
            {
                auto instruction = proc.getCurrentInstruction();
                if (instruction)
                {
                    instruction->execute(proc);
                }
                proc.setCurrentLine(proc.getCurrentLine() + 1);
                processList.updateProcess(proc);
                std::this_thread::sleep_for(std::chrono::milliseconds(delaysPerExec));
            }
            if (proc.getCurrentLine() >= proc.getLineCount())
            {
                proc.setState(ProcessState::FINISHED);
                processList.updateProcess(proc);
            }
            else
            {
                proc.setState(ProcessState::READY);
                processList.updateProcess(proc);
                std::lock_guard<std::mutex> lock(queueMutex);
                readyQueue.push(proc.getPid());
                cv.notify_one();
            }
        }
    }
}

void Scheduler::startBatchGeneration()
{
    if (batchGenerating)
        return;
    batchGenerating = true;

    batchGeneratorThread = std::thread([this]()
                                       {

        while (batchGenerating) {
            int insCount = minIns + rand() % (maxIns - minIns + 1); 

            std::vector<std::shared_ptr<Command>> cmds;

            std::string procName = "Process" + std::to_string(++processCounter);
            processList.addNewProcess(-1, 0, procName);
            int pid = processList.findProcessByName(procName);
            process &proc = processList.findProcessByRef(pid);

            std::unordered_map<std::string, int> instrCount = {
                {"DECLARE", 0},
                {"ADD", 0},
                {"SUBTRACT", 0},
                {"PRINT", 0},
                {"SLEEP", 0},
                {"FOR", 0}
            };

            std::vector<std::string> instrTypes = {
                "DECLARE", "ADD", "SUBTRACT", "PRINT", "SLEEP", "FOR"
            };

            int remaining = insCount;
            int typeIndex = 0;

            while (remaining > 0) {
                instrCount[instrTypes[typeIndex]]++;
                remaining--;
                typeIndex = (typeIndex + 1) % instrTypes.size();
            }

            int numDeclare = instrCount["DECLARE"];
            int numAdd = instrCount["ADD"];
            int numSubtract = instrCount["SUBTRACT"];
            int numPrint = instrCount["PRINT"];
            int numSleep = instrCount["SLEEP"];
            int numFor = instrCount["FOR"];

            // DECLARE
            for (int i = 0; i < numDeclare; ++i) {
                std::string varName = "var" + std::to_string(i);
                uint16_t value = rand() % 65536;  
                cmds.push_back(std::make_shared<DeclareCommand>(varName, value));
            }

            // ADD
            for (int i = 0; i < numAdd; ++i) {
                std::string var1 = "var" + std::to_string(rand() % 5);
                std::string var2 = "var" + std::to_string(rand() % 5);
                std::string var3 = "var" + std::to_string(rand() % 5);
                cmds.push_back(std::make_shared<AddCommand>(var1, var2, var3));
            }

            // SUBTRACT
            for (int i = 0; i < numSubtract; ++i) {
                std::string var1 = "var" + std::to_string(rand() % 5);
                std::string var2 = "var" + std::to_string(rand() % 5);
                std::string var3 = "var" + std::to_string(rand() % 5);
                cmds.push_back(std::make_shared<SubtractCommand>(var1, var2, var3));
            }

            // PRINT
            for (int i = 0; i < numPrint; ++i) {
                cmds.push_back(std::make_shared<PrintCommand>());
            }

            // SLEEP
            for (int i = 0; i < numSleep; ++i) {
                uint8_t sleepTime = rand() % 256;  // Sleep time between 0 and 255 (uint8)
                cmds.push_back(std::make_shared<SleepCommand>(sleepTime));
            }

            //FOR
            for (int i = 0; i < numFor; ++i)
            {
                cmds.push_back(generateForBlock(1, procName)); // Start at depth = 1
            }

            proc.clearInstructions();
            for (const auto &cmd : cmds)
            {
                proc.addInstruction(cmd);
            }

            addProcess(proc);

            std::this_thread::sleep_for(std::chrono::milliseconds(batchFreq));
        } });
}

void Scheduler::stopBatchGeneration()
{
    batchGenerating = false;
    if (batchGeneratorThread.joinable())
    {
        batchGeneratorThread.join();
    }
}