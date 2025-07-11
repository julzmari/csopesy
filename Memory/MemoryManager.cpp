#include "MemoryManager.h"
#include <algorithm>
#include <iostream> // Added for diagnostic prints

MemoryManager::MemoryManager(int totalBytes, int frameBytes)
    : totalBytes(totalBytes), frameBytes(frameBytes) {
    totalFrames = totalBytes / frameBytes;
    blocks.push_back({0, totalFrames, -1}); // all memory is free at start
}

bool MemoryManager::allocate(int processId, int bytes) {
    std::lock_guard<std::mutex> lock(mtx);
    //std::cout << "[DIAG] Attempting to allocate " << bytes << " bytes for PID " << processId << std::endl;
    int neededFrames = (bytes + frameBytes - 1) / frameBytes;
    for (auto it = blocks.begin(); it != blocks.end(); ++it) {
        //std::cout << "[DIAG] Checking block: startFrame=" << it->startFrame << ", numFrames=" << it->numFrames << ", ownerPid=" << it->ownerPid << std::endl;
        if (it->ownerPid == -1 && it->numFrames >= neededFrames) {
            // First-fit found
            int start = it->startFrame;
            // Split block if needed
            if (it->numFrames > neededFrames) {
                blocks.insert(it + 1, {start + neededFrames, it->numFrames - neededFrames, -1});
            }
            it->numFrames = neededFrames;
            it->ownerPid = processId;
            //std::cout << "[DIAG] About to call mergeFreeBlocks from allocate" << std::endl;
            // mergeFreeBlocks(); // Uncomment this line after deadlock testing
            //std::cout << "[DIAG] Allocation SUCCESS for PID " << processId << std::endl;
            return true;
        }
    }
    //std::cout << "[DIAG] Allocation FAILED for PID " << processId << std::endl;
    return false; // no suitable block
}

void MemoryManager::free(int processId) {
    std::lock_guard<std::mutex> lock(mtx);
    for (auto& block : blocks) {
        if (block.ownerPid == processId) {
            block.ownerPid = -1;
        }
    }
    mergeFreeBlocks();
}

int MemoryManager::getTotalFreeMemory() const {
    std::lock_guard<std::mutex> lock(mtx);
    int freeFrames = 0;
    for (const auto& block : blocks) {
        if (block.ownerPid == -1) freeFrames += block.numFrames;
    }
    return freeFrames * frameBytes;
}

int MemoryManager::getFramesPerProcess(int processId) const {
    std::lock_guard<std::mutex> lock(mtx);
    int frames = 0;
    for (const auto& block : blocks) {
        if (block.ownerPid == processId) frames += block.numFrames;
    }
    return frames;
}

void MemoryManager::mergeFreeBlocks() {
    if (blocks.empty()) {
        return;
    }
    std::vector<Block> merged;
    merged.push_back(blocks[0]);
    for (size_t i = 1; i < blocks.size(); ++i) {
        if (merged.back().ownerPid == -1 && blocks[i].ownerPid == -1 &&
            merged.back().startFrame + merged.back().numFrames == blocks[i].startFrame) {
            merged.back().numFrames += blocks[i].numFrames;
        } else {
            merged.push_back(blocks[i]);
        }
    }
    blocks = std::move(merged);
}

bool MemoryManager::hasEnoughMemory(int processId, int bytes) const {
    std::lock_guard<std::mutex> lock(mtx);
    int neededFrames = (bytes + frameBytes - 1) / frameBytes;
    for (const auto& block : blocks) {
        if (block.ownerPid == -1 && block.numFrames >= neededFrames) {
            return true;
        }
    }
    return false;
}

bool MemoryManager::isAllocated(int processId) const {
    std::lock_guard<std::mutex> lock(mtx);
    for (const auto& block : blocks) {
        if (block.ownerPid == processId) {
            return true;
        }
    }
    return false;
} 

