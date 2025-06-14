#ifndef CONFIG_H
#define CONFIG_H

#include <string>

enum class SchedulerAlgorithm
{
	FCFS,
	RR
};

class Config
{

private:
	int numCPU;
	SchedulerAlgorithm schedulerAlgorithm;
	int quantumCycles;
	int batchProcessFreq;
	int minIns;
	int maxIns;
	int delaysPerExec;

public:
	Config(const std::string &filename);
	void printConfig() const;

	int getNumCPU() const { return numCPU; }
	SchedulerAlgorithm getSchedulerAlgorithm() const { return schedulerAlgorithm; }
	int getQuantumCycles() const { return quantumCycles; }
	int getBatchProcessFreq() const { return batchProcessFreq; }
	int getMinIns() const { return minIns; }
	int getMaxIns() const { return maxIns; }
	int getDelaysPerExec() const { return delaysPerExec; }
};

#endif