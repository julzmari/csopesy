#include <iostream>
#include <string>
#include "process.h"

class console {
    private:
		process proc;
        int totalLines = 100;

    public:
        console();

        console(const process& proc);

        void handleScreen();
};