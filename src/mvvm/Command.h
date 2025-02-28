#pragma once

#include <memory>
#include <future>

namespace MVVM {

class Command {
public:
    virtual ~Command() {}
    virtual void execute() = 0;
    virtual bool canExecute() const { return true; }
};

class AsyncCommand : public Command {
public:
    virtual std::future<void> executeAsync() = 0;
};

} // namespace MVVM 