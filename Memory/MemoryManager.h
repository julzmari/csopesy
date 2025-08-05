#pragma once
#include <vector>
#include <utility>
#include <mutex>
#include <unordered_map>
#include <deque>
#include <fstream>
struct PageTableEntry
{
    int frameNumber;
    bool valid;
    bool dirty;
};

struct pair_hash
{
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2> &p) const
    {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        return h1 ^ h2;
    }
};

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
    uint16_t readUint16(int pid, uint32_t vaddr);
    void writeUint16(int pid, uint32_t vaddr, uint16_t value);
    int getNumPagedIn() const { return numPagedIn; }
    int getNumPagedOut() const { return numPagedOut; }
    void handlePageFault(int pid, int pageNum);
    void saveProcessToBackingStore(int pid);
    void loadProcessFromBackingStore(int pid);
    void evictProcess(int pid);
    void writeBackingStoreToFile(const std::string& filename) const;

private:
    int totalBytes;
    int frameBytes;
    int totalFrames;
    std::vector<Block> blocks; // contiguous blocks, some free, some allocated
    mutable std::mutex mtx;    // Mutex for thread-safe access, now mutable
    void mergeFreeBlocks();
    std::unordered_map<uint32_t, uint16_t> memory;
	int numPagedIn = 0;
	int numPagedOut = 0;
    std::unordered_map<int, std::vector<PageTableEntry>> pageTables;
    std::unordered_map<std::pair<int, int>, std::vector<uint8_t>, pair_hash> backingStore;
    std::deque<int> frameQueue;
    std::unordered_map<int, int> frameToPidPage;
};