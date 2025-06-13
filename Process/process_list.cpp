#include "process_list.h"

process ProcessList::findProcess(int pid) {
	auto it = processMap.find(pid);
	if (it != processMap.end()) {
		return it->second;
	}
	else {
		return process(-1, -1, -1, "Unknown", {}); // Return an invalid process if not found
	}
}

bool ProcessList::ifProcessNameExists(const std::string& processName) {
	return nameToPidMap.find(processName) != nameToPidMap.end();
}

int ProcessList::findProcessByName(const std::string& processName) {
    auto it = nameToPidMap.find(processName);
    if (it != nameToPidMap.end()) {
		return it->second;
    } else {
		return -1;
    }
}

void ProcessList::addNewProcess(int coreId, int priority, const std::string& processName, const std::vector<std::string>& instructions) {
	int newPid = ++lastPid;
	process newProc(newPid, coreId, priority, processName, instructions);
	processMap[newPid] = newProc;
	nameToPidMap[processName] = newPid;
}

void ProcessList::printAllProcesses() {
	if (processMap.empty()) {
		std::cout << "No processes found." << std::endl;
		return;
	}
	for (const auto& [pid, proc] : processMap) {
		proc.printProcessInfo();
	}
}