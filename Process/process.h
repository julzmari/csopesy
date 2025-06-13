#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sstream>

enum class ProcessState {
	NEW, READY, RUNNING, WAITING, TERMINATED
};

inline std::string getCurrentTimeString() {
	auto now = std::chrono::system_clock::now();
	std::time_t createdTime = std::chrono::system_clock::to_time_t(now);

	std::ostringstream oss;
	oss << std::put_time(std::localtime(&createdTime), "%m/%d/%Y %H:%M:%S");
	return oss.str();
}

class process {
private:
	int pid;
	int coreId;
	int priority;
	ProcessState state;
	std::string processName;
	std::vector<std::string> instructions;
	int currentLine;
	std::string creationTime;

public:
	// Constructor
	process(int pid, int coreId, int priority, const std::string& processName, const std::vector<std::string>& instructions)
		: pid(pid), coreId(coreId), priority(priority), processName(processName), instructions(instructions), currentLine(0) {
		creationTime = getCurrentTimeString();
	}

	process() : pid(0), coreId(0), priority(0), processName(""), instructions(), currentLine(0) {
		creationTime = getCurrentTimeString();
	}

	// getter setters
	int getPid() const { 
		return pid; 
	}
	int getPriority() const { 
		return priority; 
	}
	const std::string& getProcessName() const { 
		return processName; 
	}
	void setPriority(int newPriority) {
		priority = newPriority;
	}
	const std::vector<std::string>& getInstructions() const { 
		return instructions; 
	}
	int getLineCount() const { 
		return instructions.size(); 
	}
	int getCurrentLine() const { 
		return currentLine; 
	}
	std::string getCurrentInstruction() const { 
		return instructions[currentLine]; 
	}
	std::string getCreationTime() const { 
		return creationTime; 
	}

	void addInstruction(const std::string& instruction) {
		instructions.push_back(instruction);
	}
	void incrementCurrentLine() { 
		if (currentLine < instructions.size()) {
			currentLine++;
		}
	}
	bool isComplete() const { 
		return currentLine >= instructions.size(); 
	}
	void printProcessInfo() const {
		std::cout 
			<< std::left << std::setw(15)
			<< processName << std::setw(25)
			<< creationTime << std::setw(5)
			<< "Core: " << std::setw(8)
			<< coreId
			<< currentLine << " / 100\n";
	}
};