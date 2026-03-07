#include "services/TaskService.h"
#include <algorithm>

namespace taskdm {

TaskService::TaskService(TaskRepository& repository) : repository_(repository) {
}

std::optional<Task> TaskService::addTask(const std::string& title, Priority priority) {
    if (!validateTitle(title)) {
        return std::nullopt;
    }
    
    return repository_.create(title, priority);
}

std::vector<Task> TaskService::listTasks(bool include_completed) {
    return repository_.findAll(include_completed);
}

std::vector<Task> TaskService::listCompletedTasks() {
    return repository_.findCompleted();
}

bool TaskService::completeTask(int id) {
    return repository_.markCompleted(id, true);
}

bool TaskService::removeTask(int id) {
    return repository_.remove(id);
}

bool TaskService::updateTaskPriority(int id, Priority priority) {
    return repository_.updatePriority(id, priority);
}

bool TaskService::reorderTask(int draggedTaskId, int dropIndex) {
    std::vector<Task> tasks = repository_.findAll(false);
    auto it = std::find_if(tasks.begin(), tasks.end(), [draggedTaskId](const Task& t) { return t.getId() == draggedTaskId; });
    if (it == tasks.end()) return false;
    Task dragged = *it;
    tasks.erase(it);
    if (dropIndex < 0) dropIndex = 0;
    if (dropIndex > static_cast<int>(tasks.size())) dropIndex = static_cast<int>(tasks.size());
    tasks.insert(tasks.begin() + dropIndex, dragged);
    for (size_t i = 0; i < tasks.size(); ++i) {
        if (!repository_.setSortOrder(tasks[i].getId(), static_cast<int>(i))) return false;
    }
    return true;
}

std::optional<Task> TaskService::getTask(int id) {
    return repository_.findById(id);
}

bool TaskService::validateTitle(const std::string& title) {
    std::string trimmed = title;
    trimmed.erase(0, trimmed.find_first_not_of(" \t\n\r"));
    trimmed.erase(trimmed.find_last_not_of(" \t\n\r") + 1);
    
    return !trimmed.empty();
}

} 
