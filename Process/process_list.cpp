#include "process_list.h"

process ProcessList::findProcess(int pid)
{
	std::lock_guard<std::mutex> lock(mtx);
	auto it = processMap.find(pid);
	if (it != processMap.end())
	{
		return it->second;
	}
	else
	{
		return process(-1, -1, -1, "Unknown", {}, nullptr); // Return an invalid process if not found
	}
}

bool ProcessList::ifProcessNameExists(const std::string &processName)
{
	std::lock_guard<std::mutex> lock(mtx);
	return nameToPidMap.find(processName) != nameToPidMap.end();
}

int ProcessList::findProcessByName(const std::string &processName)
{
	std::lock_guard<std::mutex> lock(mtx);
	auto it = nameToPidMap.find(processName);
	if (it != nameToPidMap.end())
	{
		return it->second;
	}
	else
	{
		return -1;
	}
}

process &ProcessList::findProcessByRef(int pid)
{
	std::lock_guard<std::mutex> lock(mtx);
	auto it = processMap.find(pid);
	if (it != processMap.end())
	{
		return it->second;
	}
	else
	{
		throw std::runtime_error("Process not found");
	}
}

void ProcessList::addNewProcess(int coreId, int priority, const std::string& processName)
{
	std::lock_guard<std::mutex> lock(mtx);
	int newPid = getNextAvailablePid();
	process newProc(newPid, coreId, priority, processName, {}, nullptr); // Pass nullptr for MemoryManager  
	processMap[newPid] = newProc;
	if (nameToPidMap.find(processName) != nameToPidMap.end())
	{
		throw std::runtime_error("Process name already exists!");
	}
	nameToPidMap[processName] = newPid;
}

void ProcessList::updateProcess(const process &proc)
{
	std::lock_guard<std::mutex> lock(mtx);
	int pid = proc.getPid();
	auto it = processMap.find(pid);
	if (it != processMap.end())
	{
		it->second = proc;
	}
}

void ProcessList::removeProcess(int pid) {
    std::lock_guard<std::mutex> lock(mtx);

    processMap.erase(pid);

    for (auto it = nameToPidMap.begin(); it != nameToPidMap.end(); ) {
        if (it->second == pid) {
            it = nameToPidMap.erase(it);
        } else {
            ++it;
        }
    }
}


int ProcessList::getNextAvailablePid()
{
	return ++lastPid;
}

void ProcessList::printAllProcesses()
{
	std::lock_guard<std::mutex> lock(mtx);
	if (processMap.empty())
	{
		std::cout << "No processes found." << std::endl;
		return;
	}
	std::cout << "-----------------------------------\nRunning processes:\n";
	for (const auto &pair : processMap)
	{
		const process &proc = pair.second;
		if (proc.getState() == ProcessState::RUNNING)
		{
			proc.printProcessInfo();
		}
	}
	std::cout << "\nReady processes:\n";
	for (const auto &pair : processMap)
	{
		const process &proc = pair.second;
		if (proc.getState() == ProcessState::READY)
		{
			proc.printProcessInfo();
		}
	}

	std::cout << "\nFinished processes:\n";
	for (const auto &pair : processMap)
	{
		const process &proc = pair.second;
		if (proc.getState() == ProcessState::FINISHED)
		{
			proc.printProcessInfo();
		}
	}
	
    
}