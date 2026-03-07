#ifndef TASK_H
#define TASK_H

#include <string>
#include <chrono>
#include <ctime>

namespace taskdm {

enum class Priority {
    LOW = 0,
    MEDIUM = 1,
    HIGH = 2,
    URGENT = 3
};

class Task {
public:
    Task();
    Task(int id, const std::string& title, Priority priority, bool completed, 
         const std::string& created_at);
    
    int getId() const { return id_; }
    const std::string& getTitle() const { return title_; }
    Priority getPriority() const { return priority_; }
    bool isCompleted() const { return completed_; }
    const std::string& getCreatedAt() const { return created_at_; }
    
    void setTitle(const std::string& title) { title_ = title; }
    void setPriority(Priority priority) { priority_ = priority; }
    void setCompleted(bool completed) { completed_ = completed; }
    
    std::string priorityToString() const;
    static Priority stringToPriority(const std::string& str);
    static std::string priorityToColorString(Priority priority);
    
private:
    int id_;
    std::string title_;
    Priority priority_;
    bool completed_;
    std::string created_at_;
};

}

#endif
