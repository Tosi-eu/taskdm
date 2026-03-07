#include "repository/TaskRepository.h"
#include <iostream>
#include <cstring>
#include <string>

namespace taskdm {

TaskRepository::TaskRepository(Database& db) : db_(db) {
}

bool TaskRepository::initialize() {
    return db_.initializeSchema();
}

std::optional<Task> TaskRepository::create(const std::string& title, Priority priority) {
    if (!db_.isOpen()) {
        return std::nullopt;
    }
    
    const char* sql = "INSERT INTO tasks (title, priority, completed, sort_order) VALUES (?, ?, 0, (SELECT COALESCE(MAX(sort_order),0)+1 FROM tasks))";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        sql = "INSERT INTO tasks (title, priority, completed) VALUES (?, ?, 0)";
        if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
            std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db_.getHandle()) << std::endl;
            return std::nullopt;
        }
        sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 2, static_cast<int>(priority));
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::cerr << "Failed to insert task: " << sqlite3_errmsg(db_.getHandle()) << std::endl;
            sqlite3_finalize(stmt);
            return std::nullopt;
        }
        int id = sqlite3_last_insert_rowid(db_.getHandle());
        sqlite3_finalize(stmt);
        db_.execute("UPDATE tasks SET sort_order = " + std::to_string(id) + " WHERE id = " + std::to_string(id));
        return findById(id);
    }
    
    sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, static_cast<int>(priority));
    
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Failed to insert task: " << sqlite3_errmsg(db_.getHandle()) << std::endl;
        sqlite3_finalize(stmt);
        return std::nullopt;
    }
    
    int id = sqlite3_last_insert_rowid(db_.getHandle());
    sqlite3_finalize(stmt);
    
    return findById(id);
}

std::optional<Task> TaskRepository::findById(int id) {
    if (!db_.isOpen()) {
        return std::nullopt;
    }
    
    const char* sql = "SELECT id, title, priority, completed, created_at FROM tasks WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return std::nullopt;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        Task task = taskFromRow(stmt);
        sqlite3_finalize(stmt);
        return task;
    }
    
    sqlite3_finalize(stmt);
    return std::nullopt;
}

std::vector<Task> TaskRepository::findAll(bool include_completed) {
    std::vector<Task> tasks;
    
    if (!db_.isOpen()) {
        return tasks;
    }
    
    const char* sql = include_completed 
        ? "SELECT id, title, priority, completed, created_at FROM tasks ORDER BY COALESCE(sort_order, id) ASC, id ASC"
        : "SELECT id, title, priority, completed, created_at FROM tasks WHERE completed = 0 ORDER BY COALESCE(sort_order, id) ASC, id ASC";
    
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return tasks;
    }
    
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tasks.push_back(taskFromRow(stmt));
    }
    
    sqlite3_finalize(stmt);
    return tasks;
}

std::vector<Task> TaskRepository::findCompleted() {
    std::vector<Task> tasks;
    if (!db_.isOpen()) return tasks;
    const char* sql = "SELECT id, title, priority, completed, created_at FROM tasks WHERE completed = 1 ORDER BY COALESCE(sort_order, id) ASC, id ASC";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) return tasks;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        tasks.push_back(taskFromRow(stmt));
    }
    sqlite3_finalize(stmt);
    return tasks;
}

bool TaskRepository::update(const Task& task) {
    if (!db_.isOpen()) {
        return false;
    }
    
    const char* sql = "UPDATE tasks SET title = ?, priority = ?, completed = ? WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_text(stmt, 1, task.getTitle().c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, static_cast<int>(task.getPriority()));
    sqlite3_bind_int(stmt, 3, task.isCompleted() ? 1 : 0);
    sqlite3_bind_int(stmt, 4, task.getId());
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool TaskRepository::remove(int id) {
    if (!db_.isOpen()) {
        return false;
    }
    
    const char* sql = "DELETE FROM tasks WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool TaskRepository::markCompleted(int id, bool completed) {
    if (!db_.isOpen()) {
        return false;
    }
    
    const char* sql = "UPDATE tasks SET completed = ? WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, completed ? 1 : 0);
    sqlite3_bind_int(stmt, 2, id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool TaskRepository::updatePriority(int id, Priority priority) {
    if (!db_.isOpen()) {
        return false;
    }
    
    const char* sql = "UPDATE tasks SET priority = ? WHERE id = ?";
    sqlite3_stmt* stmt;
    
    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    
    sqlite3_bind_int(stmt, 1, static_cast<int>(priority));
    sqlite3_bind_int(stmt, 2, id);
    
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    
    return success;
}

bool TaskRepository::setSortOrder(int id, int sort_order) {
    if (!db_.isOpen()) {
        return false;
    }
    const char* sql = "UPDATE tasks SET sort_order = ? WHERE id = ?";
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db_.getHandle(), sql, -1, &stmt, nullptr) != SQLITE_OK) {
        return false;
    }
    sqlite3_bind_int(stmt, 1, sort_order);
    sqlite3_bind_int(stmt, 2, id);
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

Task TaskRepository::taskFromRow(sqlite3_stmt* stmt) {
    int id = sqlite3_column_int(stmt, 0);
    const char* title = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
    int priority = sqlite3_column_int(stmt, 2);
    int completed = sqlite3_column_int(stmt, 3);
    const char* created_at = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
    
    return Task(
        id,
        title ? std::string(title) : "",
        static_cast<Priority>(priority),
        completed == 1,
        created_at ? std::string(created_at) : ""
    );
}

}
