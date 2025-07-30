#pragma once
#include <vector>
#include <utility>
#include <mutex>
#include <unordered_map>

class MemoryManager
{
public:
    struct Block
    {
        int startFrame;
        int numFrames;
        int ownerPid; // -1 if free
    };

    MemoryManager(int totalBytes, int frameBytes);
    bool allocate(int processId, int bytes); // returns true if successful
    void free(int processId);
    int getTotalFreeMemory() const;
    int getFramesPerProcess(int processId) const;
    bool hasEnoughMemory(int processId, int bytes) const;
    bool isAllocated(int processId) const;
    int getFrameSize() const;
    std::vector<Block> getBlocksSnapshot() const;
    bool isValidAddress(uint32_t address) const;
    uint16_t readUint16(uint32_t address) const;
    void writeUint16(uint32_t address, uint16_t value);

private:
    int totalBytes;
    int frameBytes;
    int totalFrames;
    std::vector<Block> blocks; // contiguous blocks, some free, some allocated
    mutable std::mutex mtx;    // Mutex for thread-safe access, now mutable
    void mergeFreeBlocks();
    std::unordered_map<uint32_t, uint16_t> memory;
};