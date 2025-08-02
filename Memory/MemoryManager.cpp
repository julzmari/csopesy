#include "MemoryManager.h"
#include <algorithm>
#include <iostream> // Added for diagnostic prints

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
    for (auto &block : blocks)
    {
        if (block.ownerPid == processId)
        {
            block.ownerPid = -1;
        }
    }
	numPagedOut += getFramesPerProcess(processId);
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

bool MemoryManager::isValidAddress(uint32_t address) const {
    return address < static_cast<uint32_t>(totalBytes) && address % 2 == 0;
}

uint16_t MemoryManager::readUint16(uint32_t address) const {
    if (!isValidAddress(address)) {
        throw std::runtime_error("Access violation: Invalid memory address");
    }
    auto it = memory.find(address);
    if (it != memory.end()) {
        return it->second;
    }
    return 0;
}

void MemoryManager::writeUint16(uint32_t address, uint16_t value) {
    if (!isValidAddress(address)) {
        throw std::runtime_error("Access violation: Invalid memory address");
    }
    memory[address] = value;
}