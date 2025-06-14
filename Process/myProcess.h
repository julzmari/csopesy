#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sstream>
#include <memory>
#include "PrintCommand.h"

enum class ProcessState
{
	NEW,
	READY,
	RUNNING,
	WAITING,
	TERMINATED
};

inline std::string getCurrentTimeString()
{
	auto now = std::chrono::system_clock::now();
	std::time_t createdTime = std::chrono::system_clock::to_time_t(now);

	std::ostringstream oss;
	oss << std::put_time(std::localtime(&createdTime), "%m/%d/%Y %H:%M:%S");
	return oss.str();
}

class process
{
private:
	int pid;
	int coreId;
	int priority;
	ProcessState state;
	std::string processName;
	std::vector<std::shared_ptr<PrintCommand>> instructions;
	int currentLine;
	std::string creationTime;

public:
	// Constructor
	process(int pid, int coreId, int priority, const std::string &processName, const std::vector<std::shared_ptr<PrintCommand>> &instructions)
		: pid(pid), coreId(coreId), priority(priority), processName(processName), instructions(instructions), currentLine(0)
	{
		creationTime = getCurrentTimeString();
		state = ProcessState::NEW;
	}

	process() : pid(0), coreId(-1), priority(0), processName(""), currentLine(0)
	{
		creationTime = getCurrentTimeString();
		state = ProcessState::NEW;
	}

	// Getters and setters
	int getPid() const
	{
		return pid;
	}

	int getPriority() const
	{
		return priority;
	}

	const std::string &getProcessName() const
	{
		return processName;
	}

	void setState(ProcessState newState)
	{
		state = newState;
	}

	ProcessState getState() const
	{
		return state;
	}

	void setPriority(int newPriority)
	{
		priority = newPriority;
	}

	const std::vector<std::shared_ptr<PrintCommand>> &getInstructions() const
	{
		return instructions;
	}

	int getLineCount() const
	{
		return static_cast<int>(instructions.size());
	}

	int getCurrentLine() const
	{
		return currentLine;
	}

	void setCoreId(int id)
	{
		coreId = id;
	}

	void setCurrentLine(int line)
	{
		currentLine = line;
	}

	std::shared_ptr<PrintCommand> getCurrentInstruction() const
	{
		if (currentLine < instructions.size())
			return instructions[currentLine];
		return nullptr;
	}

	std::string getCreationTime() const
	{
		return creationTime;
	}

	void addInstruction(const std::shared_ptr<PrintCommand> &command)
	{
		instructions.push_back(command);
	}

	bool isComplete() const
	{
		return currentLine >= instructions.size();
	}

	void printProcessInfo() const
	{
		std::string coreIdStr = (coreId < 0) ? "N/A" : std::to_string(coreId);

		std::string stateStr;
		switch (state)
		{
		case ProcessState::NEW:
			stateStr = "NEW";
			break;
		case ProcessState::READY:
			stateStr = "READY";
			break;
		case ProcessState::RUNNING:
			stateStr = "RUNNING";
			break;
		case ProcessState::WAITING:
			stateStr = "WAITING";
			break;
		case ProcessState::TERMINATED:
			stateStr = "TERMINATED";
			break;
		default:
			stateStr = "UNKNOWN";
			break;
		}

		std::cout << std::left << std::setw(15)
				  << processName << std::setw(25)
				  << creationTime << std::setw(8)
				  << "Core: " << std::setw(5)
				  << coreIdStr << "Line: "
				  << currentLine << "/" << getLineCount() << "\n";
	}

	void executeNextInstruction(int coreId)
	{
		if (currentLine < instructions.size() && instructions[currentLine])
		{
			instructions[currentLine]->execute(coreId);
			currentLine++;
		}
	}
};
