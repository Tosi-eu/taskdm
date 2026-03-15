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
    bool toaster_enabled = true;
    int toaster_first_delay_ms = 30 * 1000;
    int toaster_interval_urgent_ms = 5 * 60 * 1000;
    int toaster_interval_high_ms = 15 * 60 * 1000;
    int toaster_interval_medium_ms = 30 * 60 * 1000;
    int toaster_interval_low_ms = 60 * 60 * 1000;
    int toaster_duration_ms = 10000;
    int toaster_position = 1;  /* 0=top-left, 1=bottom-right (default), 2=top-right, 3=bottom-left */
    bool toaster_sound_enabled = true;
    
    bool loadFromFile(const std::string& config_path);
    bool saveToFile(const std::string& config_path) const;
    
    static Config loadDefault();    
    static std::string getDefaultConfigPath();
};

}

#endif
