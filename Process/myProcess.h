#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <ctime>
#include <chrono>
#include <sstream>
#include <memory>
#include <unordered_map>
#include "Command.h"
#include "ForCommand.h"

enum class ProcessState
{
	READY,
	RUNNING,
	WAITING,
	FINISHED
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
	std::vector<std::shared_ptr<Command>> instructions;
	int currentLine = 0;
	std::string creationTime;
	std::vector<std::string> logs; // for print command
	bool isSleeping = false;
	uint8_t sleepTime = 0; // in milliseconds for sleep state

public:
	std::unordered_map<std::string, uint16_t> variables;
	// Constructor
	process(int pid, int coreId, int priority, const std::string &processName, const std::vector<std::shared_ptr<Command>> &instructions)
		: pid(pid), coreId(coreId), priority(priority), processName(processName), instructions(instructions)
	{
		creationTime = getCurrentTimeString();
		state = ProcessState::READY;
	}

	process() : pid(0), coreId(-1), priority(0), processName("")
	{
		creationTime = getCurrentTimeString();
		state = ProcessState::READY;
	}

	// methods
	void addLog(const std::stringstream &log)
	{
		logs.emplace_back(log.str());
	}
	void printLogs() const
	{
		for (const auto &log : logs)
		{
			std::cout << log;
		}
	}

	// Getters and setters
	const std::vector<std::string> &getLogs() const
	{
		return logs;
	}

	void setSleeping(bool sleeping)
	{
		isSleeping = sleeping;
	}
	uint8_t getSleepTime() const
	{
		return sleepTime;
	}
	void setSleepTime(uint8_t time)
	{
		sleepTime = time;
	}
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

	const std::vector<std::shared_ptr<Command>> &getInstructions() const
	{
		return instructions;
	}

	void clearInstructions()
	{
		instructions.clear();
	}

	int getLineCount() const
	{
		int count = 0;
		for (const auto& cmd : instructions) {
			count += getCommandLineCount(cmd);
		}
		return count;
	}

	int getCommandLineCount(const std::shared_ptr<Command>& cmd) const {
		auto forCmd = std::dynamic_pointer_cast<ForCommand>(cmd);
		if (forCmd) {
			int bodyCount = 0;
			for (const auto& inner : forCmd->instructions) {
				bodyCount += getCommandLineCount(inner);
			}
			return bodyCount * forCmd->repeats;
		}
		return 1;
	}

	int getCurrentLine() const
	{
		return currentLine;
	}

	void setCoreId(int id)
	{
		coreId = id;
	}

	int getCoreId() const
	{
		return coreId;
	}

	void setCurrentLine(int line)
	{
		currentLine = line;
	}

	std::string getCreationTime() const
	{
		return creationTime;
	}

	void addInstruction(const std::shared_ptr<Command> &command)
	{
		instructions.push_back(command);
	}

	bool isComplete() const
	{
		return currentLine >= instructions.size();
	}

	// util functions
	void printProcessInfo() const
	{
		std::string coreIdStr = (coreId < 0) ? "N/A" : std::to_string(coreId);

		if (state == ProcessState::FINISHED)
		{
			std::cout << std::left << std::setw(15)
					  << processName << std::setw(25)
					  << creationTime << std::setw(13)
					  << "Finished" << "Line: "
					  << currentLine << "/" << getLineCount() << "\n";
		}
		else
		{
			std::cout << std::left << std::setw(15)
					  << processName << std::setw(25)
					  << creationTime << std::setw(8)
					  << "Core: " << std::setw(5)
					  << coreIdStr << "Line: "
					  << currentLine << "/" << getLineCount() << "\n";
		}
	}
	std::shared_ptr<Command> getCurrentInstruction() const
	{
		if (currentLine < instructions.size())
			return instructions[currentLine];
		return nullptr;
	}
	void insertInstructions(int pos, const std::vector<std::shared_ptr<Command>> &cmds)
	{
		instructions.insert(instructions.begin() + pos, cmds.begin(), cmds.end());
	}
};
