#ifndef CLI_INTERFACE_H
#define CLI_INTERFACE_H

#include "services/TaskService.h"
#include <string>

namespace taskdm {

class CLIInterface {
public:
    explicit CLIInterface(TaskService& service);
    
    int run(int argc, char* argv[]);
    static int runConfig(int argc, char* argv[]);
    
private:
    TaskService& service_;
    
    void printTask(const Task& task);
    void printTaskList(const std::vector<Task>& tasks);
};

} 

#endif 
