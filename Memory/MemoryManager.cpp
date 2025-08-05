#include "MemoryManager.h"
#include <algorithm>
#include <iostream> // Added for diagnostic prints
#include <string>

MemoryManager::MemoryManager(int totalBytes, int frameBytes)
    : totalBytes(totalBytes), frameBytes(frameBytes)
{
    totalFrames = totalBytes / frameBytes;
    blocks.push_back({0, totalFrames, -1}); // all memory is free at start
}

bool MemoryManager::allocate(int processId, int bytes)
{
    std::lock_guard<std::mutex> lock(mtx);
    int neededFrames = (bytes + frameBytes - 1) / frameBytes;

    for (size_t i = 0; i < blocks.size(); ++i) // Use index instead of iterator
    {
        if (blocks[i].ownerPid == -1 && blocks[i].numFrames >= neededFrames)
        {
            // First-fit found
            int start = blocks[i].startFrame;

            // Split block if needed
            if (blocks[i].numFrames > neededFrames)
            {
                // Insert new block AFTER current block
                blocks.insert(blocks.begin() + i + 1,
                              {start + neededFrames, blocks[i].numFrames - neededFrames, -1});
            }

            // Update current block
            blocks[i].numFrames = neededFrames;
            blocks[i].ownerPid = processId;

            numPagedIn += neededFrames;

            return true;
        }
    }

    return false; // no suitable block
}

void MemoryManager::free(int processId)
{
    std::lock_guard<std::mutex> lock(mtx);

    int frames = 0;
    for (const auto &block : blocks)
    {
        if (block.ownerPid == processId)
            frames += block.numFrames;
    }

    for (auto &block : blocks)
    {
        if (block.ownerPid == processId)
        {
            block.ownerPid = -1;
        }
    }

    numPagedOut += frames;
    mergeFreeBlocks();
}

int MemoryManager::getTotalFreeMemory() const
{
    std::lock_guard<std::mutex> lock(mtx);
    int freeFrames = 0;
    for (const auto &block : blocks)
    {
        if (block.ownerPid == -1)
            freeFrames += block.numFrames;
    }
    return freeFrames * frameBytes;
}

int MemoryManager::getFramesPerProcess(int processId) const
{
    std::lock_guard<std::mutex> lock(mtx);
    int frames = 0;
    for (const auto &block : blocks)
    {
        if (block.ownerPid == processId)
            frames += block.numFrames;
    }
    return frames;
}

void MemoryManager::mergeFreeBlocks()
{
    if (blocks.empty())
    {
        return;
    }

    std::vector<Block> merged;
    merged.push_back(blocks[0]);

    for (size_t i = 1; i < blocks.size(); ++i)
    {
        if (merged.back().ownerPid == -1 && blocks[i].ownerPid == -1 &&
            merged.back().startFrame + merged.back().numFrames == blocks[i].startFrame)
        {
            merged.back().numFrames += blocks[i].numFrames;
        }
        else
        {
            merged.push_back(blocks[i]);
        }
    }

    blocks = std::move(merged);
}

bool MemoryManager::hasEnoughMemory(int processId, int bytes) const
{
    std::lock_guard<std::mutex> lock(mtx);
    int neededFrames = (bytes + frameBytes - 1) / frameBytes;
    for (const auto &block : blocks)
    {
        if (block.ownerPid == -1 && block.numFrames >= neededFrames)
        {
            return true;
        }
    }
    return false;
}

bool MemoryManager::isAllocated(int processId) const
{
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto &block : blocks)
    {
        if (block.ownerPid == processId)
        {
            return true;
        }
    }
    return false;
}

int MemoryManager::getFrameSize() const
{
    return frameBytes;
}

std::vector<MemoryManager::Block> MemoryManager::getBlocksSnapshot() const
{
    std::lock_guard<std::mutex> lock(mtx);
    return blocks;
}

bool MemoryManager::isValidAddress(uint32_t address) const
{
    return address < static_cast<uint32_t>(totalBytes) && address % 2 == 0;
}

uint16_t MemoryManager::readUint16(int pid, uint32_t vaddr)
{
    std::lock_guard<std::mutex> lock(mtx);

    int pageSize = frameBytes;
    int pageNum = vaddr / pageSize;
    int offset = vaddr % pageSize;

    if (pageTables[pid].size() <= (size_t)pageNum || !pageTables[pid][pageNum].valid)
    {
        handlePageFault(pid, pageNum);
    }
    int frame = pageTables[pid][pageNum].frameNumber;
    uint32_t physAddr = frame * pageSize + offset;
    return memory[physAddr];
}

void MemoryManager::writeUint16(int pid, uint32_t vaddr, uint16_t value)
{
    std::lock_guard<std::mutex> lock(mtx);

    int pageSize = frameBytes;
    int pageNum = vaddr / pageSize;
    int offset = vaddr % pageSize;

    if (pageTables[pid].size() <= (size_t)pageNum || !pageTables[pid][pageNum].valid)
    {
        handlePageFault(pid, pageNum);
    }
    int frame = pageTables[pid][pageNum].frameNumber;
    uint32_t physAddr = frame * pageSize + offset;
    memory[physAddr] = value;
    pageTables[pid][pageNum].dirty = true;
}

void MemoryManager::handlePageFault(int pid, int pageNum)
{
    int pageSize = frameBytes;
    int freeFrame = -1;

    for (auto &block : blocks)
    {
        if (block.ownerPid == pid)
        {
            for (int frame = block.startFrame; frame < block.startFrame + block.numFrames; ++frame)
            {
                bool frameInUse = false;
                if (pageTables.find(pid) != pageTables.end())
                {
                    for (const auto &entry : pageTables[pid])
                    {
                        if (entry.valid && entry.frameNumber == frame)
                        {
                            frameInUse = true;
                            break;
                        }
                    }
                }
                if (!frameInUse)
                {
                    freeFrame = frame;
                    break;
                }
            }
            if (freeFrame != -1)
                break;
        }
    }

    if (freeFrame == -1)
    {
        if (frameQueue.empty())
        {
            for (auto &block : blocks)
            {
                if (block.ownerPid == pid)
                {
                    freeFrame = block.startFrame;
                    break;
                }
            }
            if (freeFrame == -1)
                throw std::runtime_error("No frames available for process " + std::to_string(pid));
        }
        else
        {
            int victimFrame = frameQueue.front();
            frameQueue.pop_front();

            if (frameToPidPage.find(victimFrame) != frameToPidPage.end())
            {
                int victimPidPage = frameToPidPage[victimFrame];
                int victimPid = victimPidPage >> 16;
                int victimPage = victimPidPage & 0xFFFF;

                if (pageTables.find(victimPid) != pageTables.end() &&
                    victimPage < pageTables[victimPid].size() &&
                    pageTables[victimPid][victimPage].dirty)
                {
                    std::vector<uint8_t> pageData(pageSize);
                    for (int i = 0; i < pageSize; i += 2)
                    {
                        uint32_t addr = victimFrame * pageSize + i;
                        if (memory.find(addr) != memory.end())
                        {
                            pageData[i] = memory[addr] & 0xFF;
                            pageData[i + 1] = (memory[addr] >> 8) & 0xFF;
                        }
                    }
                    backingStore[{victimPid, victimPage}] = pageData;
                }

                if (pageTables.find(victimPid) != pageTables.end() &&
                    victimPage < pageTables[victimPid].size())
                {
                    pageTables[victimPid][victimPage].valid = false;
                }
                numPagedOut++;
            }

            freeFrame = victimFrame;
        }
    }

    std::vector<uint8_t> pageData(pageSize, 0);
    auto it = backingStore.find({pid, pageNum});
    if (it != backingStore.end())
    {
        pageData = it->second;
        backingStore.erase(it);
    }

    for (int i = 0; i < pageSize; i += 2)
    {
        uint32_t addr = freeFrame * pageSize + i;
        memory[addr] = pageData[i] | (pageData[i + 1] << 8);
    }

    if (pageTables[pid].size() <= (size_t)pageNum)
        pageTables[pid].resize(pageNum + 1, {-1, false, false});

    pageTables[pid][pageNum].frameNumber = freeFrame;
    pageTables[pid][pageNum].valid = true;
    pageTables[pid][pageNum].dirty = false;

    frameQueue.push_back(freeFrame);
    frameToPidPage[freeFrame] = (pid << 16) | pageNum;
    numPagedIn++;
}

void MemoryManager::saveProcessToBackingStore(int pid)
{
    std::lock_guard<std::mutex> lock(mtx);

    if (pageTables.find(pid) == pageTables.end())
        return;

    for (size_t pageNum = 0; pageNum < pageTables[pid].size(); ++pageNum)
    {
        auto &entry = pageTables[pid][pageNum];
        if (entry.valid && entry.dirty)
        {
            std::vector<uint8_t> pageData(frameBytes);
            for (int i = 0; i < frameBytes; i += 2)
            {
                uint32_t addr = entry.frameNumber * frameBytes + i;
                pageData[i] = memory[addr] & 0xFF;
                pageData[i + 1] = (memory[addr] >> 8) & 0xFF;
            }
            backingStore[{pid, static_cast<int>(pageNum)}] = pageData;
            numPagedOut++;
        }
        entry.valid = false;
    }
}

void MemoryManager::evictProcess(int pid)
{
    saveProcessToBackingStore(pid);

    auto it = std::remove_if(frameQueue.begin(), frameQueue.end(),
                             [this, pid](int frame)
                             {
                                 auto mapIt = frameToPidPage.find(frame);
                                 return mapIt != frameToPidPage.end() &&
                                        (mapIt->second >> 16) == pid;
                             });
    frameQueue.erase(it, frameQueue.end());

    for (auto it = frameToPidPage.begin(); it != frameToPidPage.end();)
    {
        if ((it->second >> 16) == pid)
            it = frameToPidPage.erase(it);
        else
            ++it;
    }
}