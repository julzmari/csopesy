#pragma once

class ICommand {
public:
    virtual void execute(int coreId) = 0;
    virtual ~ICommand() = default;
};