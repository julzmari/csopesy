#include <iostream>
#include <string>
#include "myProcess.h"

class ProcessList;

class console
{
private:
    ProcessList &processList;
    process proc;
    int totalLines = 100;

public:
    console(ProcessList &plist, const process &proc);
    void handleScreen();
};