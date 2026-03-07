#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>
#include <string>
#include <memory>

namespace taskdm {

class Database {
public:
    Database(const std::string& db_path);
    ~Database();
    
    // Non-copyable
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    
    // Movable
    Database(Database&& other) noexcept;
    Database& operator=(Database&& other) noexcept;
    
    bool isOpen() const { return db_ != nullptr; }
    sqlite3* getHandle() { return db_; }
    const sqlite3* getHandle() const { return db_; }
    
    bool execute(const std::string& sql);
    bool initializeSchema();
    
private:
    sqlite3* db_;
    std::string db_path_;
    
    void close();
};

} // namespace taskdm

#endif // DATABASE_H
