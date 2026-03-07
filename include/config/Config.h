#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <cstdint>

namespace taskdm {

struct Config {
    int widget_width = 740;
    int widget_height = 440;
    int window_x = -1;
    int window_y = -1;
    int refresh_interval_ms = 2000;
    std::string db_path = "db/tasks.db";
    
    bool loadFromFile(const std::string& config_path);
    bool saveToFile(const std::string& config_path) const;
    
    static Config loadDefault();    
    static std::string getDefaultConfigPath();
};

}

#endif
