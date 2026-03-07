#include "gui/GUIWidget.h"
#include "services/TaskService.h"
#include "repository/TaskRepository.h"
#include "database/Database.h"
#include "config/Config.h"
#include <iostream>

int main() {
    try {
        auto config = taskdm::Config::loadDefault();
        
        taskdm::Database db(config.db_path);
        if (!db.isOpen()) {
            std::cerr << "Failed to open database" << std::endl;
            return 1;
        }
        
        taskdm::TaskRepository repository(db);
        if (!repository.initialize()) {
            std::cerr << "Failed to initialize database schema" << std::endl;
            return 1;
        }
        
        taskdm::TaskService service(repository);
        
        taskdm::GUIWidget widget(service, config);
        if (!widget.initialize()) {
            std::cerr << "Failed to initialize GUI" << std::endl;
            return 1;
        }
        
        widget.run();
        widget.shutdown();
        
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
