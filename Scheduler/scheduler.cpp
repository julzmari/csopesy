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
#ifdef _WIN32
#include <direct.h>
#define mkdir(path, mode) _mkdir(path)
#endif
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
      minMemPerProc(config.getMinMemPerProc()),
      maxMemPerProc(config.getMaxMemPerProc()),
      coreAssignments(config.getNumCPU(), -1)
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
    if (proc.getPid() == -1)
        return;
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
            cv.wait(lock, [this]
                    { return !readyQueue.empty() || !running; });
            if (!running)
                break;
            pid = readyQueue.front();
            readyQueue.pop();
        }

        try
        {
            process &proc = processList.findProcessByRef(pid);
            int requiredMemory = proc.getMemorySize();
            if (!memoryManager.isAllocated(proc.getPid()))
            {
                if (!memoryManager.allocate(proc.getPid(), requiredMemory))

                {
                    proc.setState(ProcessState::READY);
                    {
                        std::lock_guard<std::mutex> lock(queueMutex);
                        readyQueue.push(proc.getPid());
                    }
                    cv.notify_one();
                    continue;
                }
            }

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

                int currentCycle;
                {
                    std::lock_guard<std::mutex> lock(queueMutex);
                    currentCycle = ++quantumCycle;
                }
                snapshotMemory(currentCycle);

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
        }

        catch (const std::exception &ex)
        {
            std::cerr << "[ERROR] Exception in worker thread (run): " << ex.what() << std::endl;
            continue;
        }
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            coreAssignments[coreId] = -1;
        }
    }
}

void Scheduler::snapshotMemory(int cycle)
{
    static std::mutex snapshotMutex;
    std::lock_guard<std::mutex> lock(snapshotMutex);

    auto now = std::chrono::system_clock::now();
    std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    std::tm *now_tm = std::localtime(&now_c);
    char timebuf[64];
    std::strftime(timebuf, sizeof(timebuf), "%m/%d/%Y %I:%M:%S%p", now_tm);

    int frameSize = memoryManager.getFrameSize();

    auto blocks = memoryManager.getBlocksSnapshot();
    int procCount = 0;
    for (const auto &block : blocks)
    {
        if (block.ownerPid != -1)
            procCount++;
    }

    int extFrag = 0;
    for (const auto &block : blocks)
    {
        if (block.ownerPid == -1 && frameSize > 0 && (block.numFrames * frameSize) < minMemPerProc)
        {
            extFrag += block.numFrames * frameSize;
        }
    }

    std::vector<std::string> memLines;
    int upper = 0;
    if (!blocks.empty() && frameSize > 0)
    {
        const auto &lastBlock = blocks.back();
        upper = lastBlock.startFrame * frameSize + lastBlock.numFrames * frameSize;
    }
    memLines.push_back("----end---- = " + std::to_string(upper));

    for (auto it = blocks.rbegin(); it != blocks.rend(); ++it)
    {
        const auto &block = *it;
        int blockUpper = 0, blockLower = 0;
        if (frameSize > 0)
        {
            blockUpper = block.startFrame * frameSize + block.numFrames * frameSize;
            blockLower = block.startFrame * frameSize;
        }
        memLines.push_back(std::to_string(blockUpper));
        if (block.ownerPid == -1)
        {
            memLines.push_back("FREE");
        }
        else
        {
            memLines.push_back("P" + std::to_string(block.ownerPid));
        }
        memLines.push_back(std::to_string(blockLower));
        memLines.push_back("");
    }
    memLines.push_back("----start---- = 0");

    struct stat st = {0};
    if (stat("memory_stamp", &st) == -1)
    {
        mkdir("memory_stamp", 0755);
    }

    std::string filename = "memory_stamp/memory_stamp_" + std::to_string(cycle) + ".txt";
    std::ofstream out(filename);
    out << "Timestamp: (" << timebuf << ")\n";
    out << "Number of processes in memory: " << procCount << "\n";
    out << "Total external fragmentation in KB: " << (extFrag / 1024) << "\n\n";

    for (const auto &line : memLines)
    {
        out << line << "\n";
    }

    out.close();
}

int countInstructions(const std::shared_ptr<Command> &cmd)
{
    auto forCmd = std::dynamic_pointer_cast<ForCommand>(cmd);
    if (forCmd)
    {
        int total = 0;
        for (const auto &nested : forCmd->getInstructions())
        {
            total += countInstructions(nested);
        }
        return total * forCmd->getRepeatCount();
    }
    return 1;
}

void Scheduler::startBatchGeneration()
{
    if (batchGenerating)
        return;
    batchGenerating = true;
    batchGeneratorThread = std::thread([this]()
                                       {
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

                processList.withProcessByRef(pid, [&](process &proc) {
                    int minExp = static_cast<int>(log2(minMemPerProc));
                    int maxExp = static_cast<int>(log2(maxMemPerProc));
                    int randExp = minExp + (rand() % (maxExp - minExp + 1));
                    int memSize = 1 << randExp;

                    proc.setMemorySize(memSize);
                    proc.setMemoryManager(&memoryManager);


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

                    int totalIns = 0;
                    cmds.clear();

                    for (int i = 0; i < instrCount["DECLARE"] && totalIns < maxIns; ++i) {
                        std::string varName = "var" + std::to_string(i);
                        uint16_t value = rand() % 65536;
                        cmds.push_back(std::make_shared<DeclareCommand>(varName, value));
                        totalIns += 1;
                    }
                    
                    for (int i = 0; i < instrCount["ADD"] && totalIns < maxIns; ++i) {
                        std::string var1 = "var" + std::to_string(rand() % 5);
                        std::string var2 = "var" + std::to_string(rand() % 5);
                        std::string var3 = "var" + std::to_string(rand() % 5);
                        cmds.push_back(std::make_shared<AddCommand>(var1, var2, var3));
                        totalIns += 1;
                    }

                    for (int i = 0; i < instrCount["SUBTRACT"] && totalIns < maxIns; ++i) {
                        std::string var1 = "var" + std::to_string(rand() % 5);
                        std::string var2 = "var" + std::to_string(rand() % 5);
                        std::string var3 = "var" + std::to_string(rand() % 5);
                        cmds.push_back(std::make_shared<SubtractCommand>(var1, var2, var3));
                        totalIns += 1;
                    }

                    for (int i = 0; i < instrCount["PRINT"] && totalIns < maxIns; ++i) {
                        cmds.push_back(std::make_shared<PrintCommand>());
                        totalIns += 1;
                    }

                    for (int i = 0; i < instrCount["SLEEP"] && totalIns < maxIns; ++i) {
                        uint8_t sleepTime = rand() % 256;
                        cmds.push_back(std::make_shared<SleepCommand>(sleepTime));
                        totalIns += 1;
                    }

                    for (int i = 0; i < instrCount["FOR"] && totalIns < maxIns; ++i) {
                        auto forCmd = generateForBlock(1, procName);
                        int forIns = countInstructions(forCmd);
                        if (totalIns + forIns > maxIns) break;
                        cmds.push_back(forCmd);
                        totalIns += forIns;
                    }

                    while (totalIns < maxIns)
                    {
                        cmds.push_back(std::make_shared<PrintCommand>());
                        totalIns += 1;
                    }

                    proc.clearInstructions();
                    for (const auto &cmd : cmds) {
                        proc.addInstruction(cmd);
                    }
                    addProcess(proc);
                });

                std::this_thread::sleep_for(std::chrono::milliseconds(batchFreq));
            } catch (const std::exception& ex) {
                std::cerr << "[ERROR] Exception in batch generator: " << ex.what() << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            } catch (...) {
                std::cerr << "[ERROR] Unknown exception in batch generator" << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            }
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