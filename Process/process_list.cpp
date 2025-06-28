#include "process_list.h"

process ProcessList::findProcess(int pid)
{
	auto it = processMap.find(pid);
	if (it != processMap.end())
	{
		return it->second;
	}
	else
	{
		return process(-1, -1, -1, "Unknown", {}); // Return an invalid process if not found
	}
}

bool ProcessList::ifProcessNameExists(const std::string &processName)
{
	return nameToPidMap.find(processName) != nameToPidMap.end();
}

int ProcessList::findProcessByName(const std::string &processName)
{
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

void ProcessList::addNewProcess(int coreId, int priority, const std::string &processName)
{
	int newPid = getNextAvailablePid();
	process newProc(newPid, coreId, priority, processName, {});
	processMap[newPid] = newProc;
	if (nameToPidMap.find(processName) != nameToPidMap.end())
	{
		throw std::runtime_error("Process name already exists!");
	}
	nameToPidMap[processName] = newPid;
}

void ProcessList::updateProcess(const process &proc)
{
	int pid = proc.getPid();
	auto it = processMap.find(pid);
	if (it != processMap.end())
	{
		it->second = proc;
	}
}

int ProcessList::getNextAvailablePid()
{
	return ++lastPid;
}

void ProcessList::printAllProcesses()
{
	if (processMap.empty())
	{
		std::cout << "No processes found." << std::endl;
		return;
	}
	std::cout << "-----------------------------------\nRunning processes:\n";
	for (const auto &[pid, proc] : processMap)
	{
		if (proc.getState() == ProcessState::RUNNING)
		{
			proc.printProcessInfo();
		}
	}

	std::cout << "\nFinished processes:\n";
	for (const auto &[pid, proc] : processMap)
	{
		if (proc.getState() == ProcessState::FINISHED)
		{
			proc.printProcessInfo();
		}
	}
	std::cout << "-----------------------------------";
}