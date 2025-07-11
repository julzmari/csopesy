#include <thread>
#include "scheduler.h"
#include "PrintCommand.h"
#include "AddCommand.h"
#include "SubtractCommand.h"
#include "DeclareCommand.h"
#include "SleepCommand.h"
#include "ForCommand.h"
#include "Config.h"
#include "MemoryManager.h"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <ctime>
#include <sys/stat.h>
#include "process_list.h"

using namespace std;

Scheduler::Scheduler(ProcessList &plist, Config &config, MemoryManager &memManager)
    : processList(plist),
      numCores(config.getNumCPU()),
      running(false),
      batchFreq(config.getBatchProcessFreq()),
      minIns(config.getMinIns()),
      maxIns(config.getMaxIns()),
      delaysPerExec(config.getDelaysPerExec()),
      schedulerType(config.getSchedulerAlgorithm()),
      quantum(config.getQuantumCycles()),
      memoryManager(memManager),
      memPerProc(config.getMemPerProc()),
      coreAssignments(config.getNumCPU(), -1) // initialize all to -1
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
    if (proc.getPid() == -1) return;
    {
        std::lock_guard<std::mutex> lock(queueMutex);
        readyQueue.push(proc.getPid());
    }
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
        int pid = -1;
        {
            std::unique_lock<std::mutex> lock(queueMutex);
            cv.wait(lock, [this] { return !readyQueue.empty() || !running; });
            if (!running) break;
            pid = readyQueue.front();
            readyQueue.pop();
        } // lock released here
        // Now do all process work outside the lock
        try {
            process& proc = processList.findProcessByRef(pid);
            proc.setState(ProcessState::RUNNING);
            proc.setCoreId(coreId);
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
                    std::this_thread::sleep_for(std::chrono::milliseconds(delaysPerExec));
                }
                proc.setState(ProcessState::FINISHED);
                memoryManager.free(proc.getPid());
            }
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
                    std::this_thread::sleep_for(std::chrono::milliseconds(delaysPerExec));
                }
                quantumCycle++;
                snapshotMemory(quantumCycle);
                if (proc.getCurrentLine() >= proc.getLineCount())
                {
                    proc.setState(ProcessState::FINISHED);
                    memoryManager.free(proc.getPid());
                }
                else
                {
                    proc.setState(ProcessState::READY);
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        readyQueue.push(proc.getPid());
                    }
                    cv.notify_one();
                }
            }
        } catch (const std::exception& ex) {
            std::cerr << "[ERROR] Exception in worker thread (run): " << ex.what() << std::endl;
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            coreAssignments[coreId] = -1;
        }
    }
}

void Scheduler::snapshotMemory(int cycle) {
    // Get timestamp
    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm* now_tm = std::localtime(&now_c);
    char timebuf[64];
    std::strftime(timebuf, sizeof(timebuf), "%m/%d/%Y %I:%M:%S%p", now_tm);

    // Count processes in memory
    int procCount = 0;
    for (const auto& block : memoryManager.getBlocks()) {
        if (block.ownerPid != -1) procCount++;
    }

    // Calculate external fragmentation (sum of all free blocks < mem-per-proc)
    int extFrag = 0;
    int memPerProc = memoryManager.getBlocks().empty() ? 4096 : this->memPerProc;
    for (const auto& block : memoryManager.getBlocks()) {
        if (block.ownerPid == -1 && (block.numFrames * memoryManager.getBlocks()[0].numFrames) < memPerProc) {
            extFrag += block.numFrames * memoryManager.getBlocks()[0].numFrames;
        }
    }

    // Prepare ASCII memory printout
    std::vector<std::string> memLines;
    int upper = memoryManager.getBlocks().empty() ? 0 : memoryManager.getBlocks().back().startFrame * memoryManager.getBlocks()[0].numFrames + memoryManager.getBlocks().back().numFrames * memoryManager.getBlocks()[0].numFrames;
    int lower = 0;
    memLines.push_back("----end---- = " + std::to_string(upper));
    for (auto it = memoryManager.getBlocks().rbegin(); it != memoryManager.getBlocks().rend(); ++it) {
        const auto& block = *it;
        int blockUpper = block.startFrame * memoryManager.getBlocks()[0].numFrames + block.numFrames * memoryManager.getBlocks()[0].numFrames;
        int blockLower = block.startFrame * memoryManager.getBlocks()[0].numFrames;
        memLines.push_back(std::to_string(blockUpper));
        if (block.ownerPid == -1) {
            memLines.push_back("FREE");
        } else {
            memLines.push_back("P" + std::to_string(block.ownerPid));
        }
        memLines.push_back(std::to_string(blockLower));
    }
    memLines.push_back("----start---- = 0");

    // Ensure the memory_stamp directory exists
    struct stat st = {0};
    if (stat("memory_stamp", &st) == -1) {
        mkdir("memory_stamp", 0755);
    }
    std::string filename = "memory_stamp/memory_stamp_" + std::to_string(cycle) + ".txt";
    std::ofstream out(filename);
    out << "Timestamp: (" << timebuf << ")\n";
    out << "Number of processes in memory: " << procCount << "\n";
    out << "Total external fragmentation in KB: " << (extFrag / 1024) << "\n\n";
    for (const auto& line : memLines) {
        out << line << "\n";
    }
    out.close();
}

void Scheduler::startBatchGeneration()
{
    if (batchGenerating)
        return;
    batchGenerating = true;
    batchGeneratorThread = std::thread([this]() {
        while (batchGenerating) {
            try {
                int insCount = minIns + rand() % (maxIns - minIns + 1);
                std::vector<std::shared_ptr<Command>> cmds;
                std::string procName = "Process" + std::to_string(++processCounter);
                
                processList.addNewProcess(-1, 0, procName);
                int pid = processList.findProcessByName(procName);
                
                if (pid == -1) {
                    std::cerr << "[ERROR] Failed to find process by name: " << procName << std::endl;
                    continue;
                }
                
                // Use the safe withProcessByRef method instead of direct pointer manipulation
                processList.withProcessByRef(pid, [&](process& proc) {
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
                    for (int i = 0; i < numDeclare; ++i) {
                        std::string varName = "var" + std::to_string(i);
                        uint16_t value = rand() % 65536;
                        cmds.push_back(std::make_shared<DeclareCommand>(varName, value));
                    }
                    for (int i = 0; i < numAdd; ++i) {
                        std::string var1 = "var" + std::to_string(rand() % 5);
                        std::string var2 = "var" + std::to_string(rand() % 5);
                        std::string var3 = "var" + std::to_string(rand() % 5);
                        cmds.push_back(std::make_shared<AddCommand>(var1, var2, var3));
                    }
                    for (int i = 0; i < numSubtract; ++i) {
                        std::string var1 = "var" + std::to_string(rand() % 5);
                        std::string var2 = "var" + std::to_string(rand() % 5);
                        std::string var3 = "var" + std::to_string(rand() % 5);
                        cmds.push_back(std::make_shared<SubtractCommand>(var1, var2, var3));
                    }
                    for (int i = 0; i < numPrint; ++i) {
                        cmds.push_back(std::make_shared<PrintCommand>());
                    }
                    for (int i = 0; i < numSleep; ++i) {
                        uint8_t sleepTime = rand() % 256;
                        cmds.push_back(std::make_shared<SleepCommand>(sleepTime));
                    }
                    for (int i = 0; i < numFor; ++i)
                    {
                        cmds.push_back(generateForBlock(1, procName));
                    }
                    proc.clearInstructions();
                    for (const auto &cmd : cmds)
                    {
                        proc.addInstruction(cmd);
                    }
                    addProcess(proc);
                });
                
                std::this_thread::sleep_for(std::chrono::milliseconds(batchFreq));
            } catch (const std::exception& ex) {
                std::cerr << "[ERROR] Exception in batch generator: " << ex.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Wait a bit before retrying
            } catch (...) {
                std::cerr << "[ERROR] Unknown exception in batch generator" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Wait a bit before retrying
            }
        }
    });
}

void Scheduler::stopBatchGeneration()
{
    batchGenerating = false;
    if (batchGeneratorThread.joinable())
    {
        batchGeneratorThread.join();
    }
}