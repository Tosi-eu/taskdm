#include "cli/CLIInterface.h"
#include "config/Config.h"
#include "core/Task.h"
#include <iostream>
#include <iomanip>
#include <ctime>
#include <cstdio>
namespace taskdm {

int CLIInterface::runConfig(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: task-cli config <get|set> [options]" << std::endl;
        std::cout << "  config get                    Show current config" << std::endl;
        std::cout << "  config set <key> <value>       Set widget_width, widget_height" << std::endl;
        return 1;
    }
    std::string sub = argv[2];
    Config config = Config::loadDefault();
    std::string config_path = Config::getDefaultConfigPath();

    if (sub == "get") {
        std::cout << "window_x: " << config.window_x << "  window_y: " << config.window_y << std::endl;
        std::cout << "widget_width: " << config.widget_width << std::endl;
        std::cout << "widget_height: " << config.widget_height << std::endl;
        std::cout << "refresh_interval_ms: " << config.refresh_interval_ms << std::endl;
        std::cout << "db_path: " << config.db_path << std::endl;
        std::cout << "config file: " << config_path << std::endl;
        return 0;
    }

    if (sub == "set") {
        if (argc < 5) {
            std::cerr << "Usage: task-cli config set <key> <value>" << std::endl;
            std::cerr << "  e.g. task-cli config set position top-right" << std::endl;
            return 1;
        }
        std::string key = argv[3];
        std::string value = argv[4];
        if (key == "widget_width" || key == "width") {
            try {
                int w = std::stoi(value);
                if (w > 0) config.widget_width = w;
            } catch (...) {
                std::cerr << "Invalid number for widget_width" << std::endl;
                return 1;
            }
        } else if (key == "widget_height" || key == "height") {
            try {
                int h = std::stoi(value);
                if (h > 0) config.widget_height = h;
            } catch (...) {
                std::cerr << "Invalid number for widget_height" << std::endl;
                return 1;
            }
        } else {
            std::cerr << "Unknown key. Supported: widget_width, widget_height" << std::endl;
            return 1;
        }
        if (config.saveToFile(config_path)) {
            std::cout << "Config saved to " << config_path << std::endl;
            return 0;
        }
        std::cerr << "Failed to save config" << std::endl;
        return 1;
    }

    std::cerr << "Unknown subcommand. Use: config get | config set <key> <value>" << std::endl;
    return 1;
}

CLIInterface::CLIInterface(TaskService& service) : service_(service) {
}

void CLIInterface::printTask(const Task& task) {
    std::string status = task.isCompleted() ? "[X]" : "[ ]";
    std::string priority_str = task.priorityToString();
    
    std::cout << std::setw(4) << task.getId() << " "
              << status << " "
              << std::setw(8) << std::left << priority_str << " "
              << task.getTitle() << std::endl;
}

void CLIInterface::printTaskList(const std::vector<Task>& tasks) {
    if (tasks.empty()) {
        std::cout << "No tasks found." << std::endl;
        return;
    }
    
    std::cout << std::setw(4) << "ID" << " "
              << "STATUS" << " "
              << std::setw(8) << std::left << "PRIORITY" << " "
              << "TITLE" << std::endl;
    std::cout << std::string(80, '-') << std::endl;
    
    for (const auto& task : tasks) {
        printTask(task);
    }
}

int CLIInterface::run(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: task-cli <command> [options]" << std::endl;
        std::cout << "Commands:" << std::endl;
        std::cout << "  add \"<title>\" [--priority <level>]  Add a new task" << std::endl;
        std::cout << "  list                                 List all active tasks" << std::endl;
        std::cout << "  done <id>                            Mark task as completed" << std::endl;
        std::cout << "  remove <id>                          Remove a task" << std::endl;
        std::cout << "  priority <id> <level>                Update task priority" << std::endl;
        std::cout << "  config get|set [key] [value]          Get or set widget/config options" << std::endl;
        std::cout << "  history [YYYY-MM-DD]                    List completed tasks on one day (default: today)" << std::endl;
        std::cout << "  history YYYY-MM-DD YYYY-MM-DD           List completed tasks in date range" << std::endl;
        std::cout << "Priority levels: LOW, MEDIUM, HIGH, URGENT" << std::endl;
        return 1;
    }
    
    std::string command = argv[1];
    if (command == "config") {
        return runConfig(argc, argv);
    }
    if (command == "add") {
        if (argc < 3) {
            std::cerr << "Error: Task title required" << std::endl;
            return 1;
        }
        
        std::string title = argv[2];
        Priority priority = Priority::MEDIUM;
        
        for (int i = 3; i < argc; i++) {
            if (std::string(argv[i]) == "--priority" && i + 1 < argc) {
                priority = Task::stringToPriority(argv[i + 1]);
                break;
            }
        }
        
        auto task = service_.addTask(title, priority);
        if (task) {
            std::cout << "Task added successfully (ID: " << task->getId() << ")" << std::endl;
            return 0;
        } else {
            std::cerr << "Error: Failed to add task" << std::endl;
            return 1;
        }
    }
    else if (command == "list") {
        auto tasks = service_.listTasks(false);
        printTaskList(tasks);
        return 0;
    }
    else if (command == "history") {
        std::time_t now = std::time(nullptr);
        std::tm* tm = std::localtime(&now);
        char today_buf[32];
        std::snprintf(today_buf, sizeof(today_buf), "%04d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
        std::string today_str(today_buf);
        if (argc >= 4) {
            std::string from_str = argv[2];
            std::string to_str = argv[3];
            if (from_str > to_str) std::swap(from_str, to_str);
            auto tasks = service_.listCompletedTasksByDateRange(from_str, to_str);
            std::cout << "Completed between " << from_str << " and " << to_str << ":" << std::endl;
            printTaskList(tasks);
        } else {
            std::string date_arg = (argc >= 3) ? argv[2] : today_str;
            auto tasks = service_.listCompletedTasksByDate(date_arg);
            std::cout << "Completed on " << date_arg << ":" << std::endl;
            printTaskList(tasks);
        }
        return 0;
    }
    else if (command == "done") {
        if (argc < 3) {
            std::cerr << "Error: Task ID required" << std::endl;
            return 1;
        }
        
        int id = std::stoi(argv[2]);
        if (service_.completeTask(id)) {
            std::cout << "Task " << id << " marked as completed" << std::endl;
            return 0;
        } else {
            std::cerr << "Error: Failed to complete task" << std::endl;
            return 1;
        }
    }
    else if (command == "remove") {
        if (argc < 3) {
            std::cerr << "Error: Task ID required" << std::endl;
            return 1;
        }
        
        int id = std::stoi(argv[2]);
        if (service_.removeTask(id)) {
            std::cout << "Task " << id << " removed" << std::endl;
            return 0;
        } else {
            std::cerr << "Error: Failed to remove task" << std::endl;
            return 1;
        }
    }
    else if (command == "priority") {
        if (argc < 4) {
            std::cerr << "Error: Task ID and priority level required" << std::endl;
            return 1;
        }
        
        int id = std::stoi(argv[2]);
        Priority priority = Task::stringToPriority(argv[3]);
        
        if (service_.updateTaskPriority(id, priority)) {
            std::cout << "Task " << id << " priority updated to " << argv[3] << std::endl;
            return 0;
        } else {
            std::cerr << "Error: Failed to update priority" << std::endl;
            return 1;
        }
    }
    else {
        std::cerr << "Unknown command: " << command << std::endl;
        return 1;
    }
}

}
