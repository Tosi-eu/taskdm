#ifndef TASK_SERVICE_H
#define TASK_SERVICE_H

#include "repository/TaskRepository.h"
#include "core/Task.h"
#include <vector>
#include <optional>

namespace taskdm {

class TaskService {
public:
    explicit TaskService(TaskRepository& repository);
    
    std::optional<Task> addTask(const std::string& title, Priority priority);
    std::vector<Task> listTasks(bool include_completed = false);
    std::vector<Task> listCompletedTasks();
    std::vector<Task> listCompletedTasksByDate(const std::string& date_yyyy_mm_dd);
    bool completeTask(int id);
    bool removeTask(int id);
    bool updateTaskPriority(int id, Priority priority);
    bool reorderTask(int draggedTaskId, int dropIndex);
    
    std::optional<Task> getTask(int id);
    
private:
    TaskRepository& repository_;
    
    bool validateTitle(const std::string& title);
};

} 

#endif 
