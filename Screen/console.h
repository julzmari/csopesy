#include <iostream>
#include <string>
#include "myProcess.h"

class console {
    private:
		process proc;
        int totalLines = 100;

    public:
        console();

        console(const process& proc);

        void handleScreen();
};