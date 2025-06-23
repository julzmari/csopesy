#pragma once  
class Command {  
public:  
    virtual void execute(process& context) = 0;  
    virtual std::shared_ptr<Command> clone() const = 0;
    virtual ~Command() = default;  
};