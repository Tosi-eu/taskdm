#include "core/Task.h"
#include <algorithm>
#include <cctype>

namespace taskdm {

Task::Task() 
    : id_(0), priority_(Priority::MEDIUM), completed_(false), created_at_("") {
}

Task::Task(int id, const std::string& title, Priority priority, bool completed,
           const std::string& created_at)
    : id_(id), title_(title), priority_(priority), completed_(completed),
      created_at_(created_at) {
}

std::string Task::priorityToString() const {
    switch (priority_) {
        case Priority::LOW: return "LOW";
        case Priority::MEDIUM: return "MEDIUM";
        case Priority::HIGH: return "HIGH";
        case Priority::URGENT: return "URGENT";
        default: return "MEDIUM";
    }
}

Priority Task::stringToPriority(const std::string& str) {
    std::string upper = str;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
    
    if (upper == "LOW") return Priority::LOW;
    if (upper == "MEDIUM") return Priority::MEDIUM;
    if (upper == "HIGH") return Priority::HIGH;
    if (upper == "URGENT") return Priority::URGENT;
    
    return Priority::MEDIUM; // Default
}

std::string Task::priorityToColorString(Priority priority) {
    switch (priority) {
        case Priority::LOW: return "gray";
        case Priority::MEDIUM: return "yellow";
        case Priority::HIGH: return "orange";
        case Priority::URGENT: return "red";
        default: return "yellow";
    }
}

} // namespace taskdm
