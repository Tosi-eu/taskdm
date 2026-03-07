#include "cli/CLIInterface.h"
#include "services/TaskService.h"
#include "repository/TaskRepository.h"
#include "database/Database.h"
#include "config/Config.h"
#include <iostream>

int main(int argc, char* argv[]) {
    try {
        if (argc >= 2 && std::string(argv[1]) == "config") {
            return taskdm::CLIInterface::runConfig(argc, argv);
        }
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

        taskdm::CLIInterface cli(service);
        return cli.run(argc, argv);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
