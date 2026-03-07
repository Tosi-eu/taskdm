#include "config/Config.h"
#include <fstream>
#include <iostream>
#include <filesystem>
#include <cstdlib>

namespace taskdm {

namespace {
    std::string getHomeDir() {
        const char* home = std::getenv("HOME");
        return home ? std::string(home) : "";
    }
    std::string getXdgConfigPath() {
        std::string home = getHomeDir();
        if (home.empty()) return "";
        return home + "/.config/taskdm/config.json";
    }
    std::string getXdgDataPath() {
        std::string home = getHomeDir();
        if (home.empty()) return "";
        return home + "/.local/share/taskdm/tasks.db";
    }
}

bool Config::loadFromFile(const std::string& config_path) {
    std::ifstream file(config_path);
    if (!file.is_open()) {
        return false;
    }
    
    std::string content((std::istreambuf_iterator<char>(file)),
                        std::istreambuf_iterator<char>());
    file.close();
    
    auto findValue = [&content](const std::string& key) -> std::string {
        size_t pos = content.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        pos = content.find(":", pos);
        if (pos == std::string::npos) return "";
        pos = content.find_first_of("\"", pos + 1);
        if (pos == std::string::npos) return "";
        size_t end = content.find("\"", pos + 1);
        if (end == std::string::npos) return "";
        return content.substr(pos + 1, end - pos - 1);
    };
    
    auto findIntValue = [&content](const std::string& key) -> int {
        size_t pos = content.find("\"" + key + "\"");
        if (pos == std::string::npos) return -1;
        pos = content.find(":", pos);
        if (pos == std::string::npos) return -1;
        pos = content.find_first_not_of(" \t", pos + 1);
        if (pos == std::string::npos) return -1;
        size_t end = content.find_first_of(",}\n", pos);
        if (end == std::string::npos) return -1;
        std::string val = content.substr(pos, end - pos);
        try {
            return std::stoi(val);
        } catch (...) {
            return -1;
        }
    };
    
    int wx = findIntValue("window_x");
    if (wx >= 0) window_x = wx;
    int wy = findIntValue("window_y");
    if (wy >= 0) window_y = wy;

    int width = findIntValue("widget_width");
    if (width > 0) widget_width = width;
    
    int height = findIntValue("widget_height");
    if (height > 0) widget_height = height;
    
    int interval = findIntValue("refresh_interval_ms");
    if (interval > 0) refresh_interval_ms = interval;
    
    std::string db_path_val = findValue("db_path");
    if (!db_path_val.empty()) db_path = db_path_val;
    
    return true;
}

bool Config::saveToFile(const std::string& config_path) const {
    std::filesystem::path path(config_path);
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path());
    }
    
    std::ofstream file(config_path);
    if (!file.is_open()) {
        return false;
    }
    
    file << "{\n";
    file << "  \"widget_width\": " << widget_width << ",\n";
    file << "  \"widget_height\": " << widget_height << ",\n";
    file << "  \"window_x\": " << window_x << ",\n";
    file << "  \"window_y\": " << window_y << ",\n";
    file << "  \"refresh_interval_ms\": " << refresh_interval_ms << ",\n";
    file << "  \"db_path\": \"" << db_path << "\"\n";
    file << "}\n";
    
    return true;
}

std::string Config::getDefaultConfigPath() {
    std::string xdg = getXdgConfigPath();
    if (!xdg.empty() && std::filesystem::exists(xdg))
        return xdg;
    if (std::filesystem::exists("config/config.json"))
        return "config/config.json";
    if (std::filesystem::exists("../config/config.json"))
        return "../config/config.json";
    return xdg.empty() ? "config/config.json" : xdg;
}

Config Config::loadDefault() {
    Config config;
    std::string config_path = getDefaultConfigPath();
    if (std::filesystem::exists(config_path))
        config.loadFromFile(config_path);
    bool use_xdg = (config_path == getXdgConfigPath());
    if (use_xdg || (!std::filesystem::exists("config/config.json") && !std::filesystem::exists("../config/config.json") && !std::filesystem::exists("db"))) {
        std::string xdg_db = getXdgDataPath();
        if (!xdg_db.empty()) {
            config.db_path = xdg_db;
            std::filesystem::create_directories(std::filesystem::path(xdg_db).parent_path());
        }
    }
    return config;
}

} 
