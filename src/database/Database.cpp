#include "database/Database.h"
#include <iostream>
#include <filesystem>
#include <cstring>

namespace taskdm {

Database::Database(const std::string& db_path) : db_(nullptr), db_path_(db_path) {
    std::filesystem::path path(db_path);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }
    
    int rc = sqlite3_open(db_path.c_str(), &db_);
    if (rc != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db_) << std::endl;
        db_ = nullptr;
    }
}

Database::~Database() {
    close();
}

Database::Database(Database&& other) noexcept 
    : db_(other.db_), db_path_(std::move(other.db_path_)) {
    other.db_ = nullptr;
}

Database& Database::operator=(Database&& other) noexcept {
    if (this != &other) {
        close();
        db_ = other.db_;
        db_path_ = std::move(other.db_path_);
        other.db_ = nullptr;
    }
    return *this;
}

void Database::close() {
    if (db_) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

bool Database::execute(const std::string& sql) {
    if (!db_) {
        return false;
    }
    
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err_msg);
    
    if (rc != SQLITE_OK) {
        std::cerr << "SQL error: " << err_msg << std::endl;
        sqlite3_free(err_msg);
        return false;
    }
    
    return true;
}

bool Database::initializeSchema() {
    if (!db_) {
        return false;
    }
    
    const char* schema = R"(
        CREATE TABLE IF NOT EXISTS tasks (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            priority INTEGER NOT NULL,
            completed INTEGER NOT NULL DEFAULT 0,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP
        );
    )";
    if (!execute(schema)) {
        return false;
    }
    char* err_msg = nullptr;
    int rc = sqlite3_exec(db_, "ALTER TABLE tasks ADD COLUMN sort_order INTEGER", nullptr, nullptr, &err_msg);
    if (rc != SQLITE_OK && err_msg) {
        if (std::strstr(err_msg, "duplicate column") == nullptr) {
            std::cerr << "SQL error: " << err_msg << std::endl;
            sqlite3_free(err_msg);
            return false;
        }
        sqlite3_free(err_msg);
    }
    if (!execute("UPDATE tasks SET sort_order = id WHERE sort_order IS NULL")) {
        return false;
    }
    return true;
}

} 
