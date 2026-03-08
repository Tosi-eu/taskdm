#ifndef TASK_REPOSITORY_H
#define TASK_REPOSITORY_H

#include "core/Task.h"
#include "database/Database.h"
#include <vector>
#include <memory>
#include <optional>

namespace taskdm {

class TaskRepository {
public:
    explicit TaskRepository(Database& db);
    
    bool initialize();
    
    std::optional<Task> create(const std::string& title, Priority priority);
    std::optional<Task> findById(int id);
    std::vector<Task> findAll(bool include_completed = false);
    std::vector<Task> findCompleted();
    std::vector<Task> findCompletedByDate(const std::string& date_yyyy_mm_dd);
    bool update(const Task& task);
    bool remove(int id);
    
    bool markCompleted(int id, bool completed);
    bool updatePriority(int id, Priority priority);
    bool setSortOrder(int id, int sort_order);
    
private:
    Database& db_;
    
    Task taskFromRow(sqlite3_stmt* stmt);
};

}

#endif
