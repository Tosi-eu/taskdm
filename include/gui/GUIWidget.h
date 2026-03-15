#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "services/TaskService.h"
#include "config/Config.h"
#include "core/Task.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <chrono>
#include <string>

struct ImGuiContext;
struct ImVec4;

namespace taskdm {

class GUIWidget {
public:
    GUIWidget(TaskService& service, const Config& config);
    ~GUIWidget();
    
    bool initialize();
    void run();
    void shutdown();
    
private:
    TaskService& service_;
    Config config_;
    GLFWwindow* window_;
    GLFWwindow* toaster_window_ = nullptr;
    ImGuiContext* imgui_context_;
    
    std::vector<Task> current_tasks_;
    std::vector<Task> finished_tasks_;
    std::vector<Task> history_tasks_;
    std::string history_date_;
    bool history_use_range_ = false;
    char history_from_buf_[32] = {};
    char history_to_buf_[32] = {};
    std::chrono::steady_clock::time_point last_refresh_;
    std::chrono::steady_clock::time_point last_config_check_;
    struct DragState {
        bool dragging = false;
        double offset_x = 0;
        double offset_y = 0;
        int start_win_x = 0;
        int start_win_y = 0;
    } drag_state_;
    bool mouse_left_down_prev_ = false;
    bool show_add_form_ = false;
    char new_task_buf_[256] = {};
    int new_task_priority_idx_ = 1;
    unsigned int check_tex_ = 0;
    unsigned int error_tex_ = 0;
    bool use_native_title_bar_ = false;
    bool toaster_visible_ = false;
    bool toaster_has_shown_once_ = false;
    std::chrono::steady_clock::time_point toaster_show_start_;
    std::chrono::steady_clock::time_point last_toaster_time_;
    float toaster_shake_time_ = 0.f;

    int getToasterIntervalMsForPriority(Priority p) const;
    int getNextToasterIntervalMs() const;
    static bool isWayland();
    void playNotificationSound();
    void sendWaylandNotification();
    void ensureToasterWindow();
    void positionToasterOnMonitor(float shake_x);
    void renderToaster();

    void updateTasks();
    void updateHistoryTasks();
    void updateHistoryTasksRange();
    std::string getTodayDate() const;
    std::string getPrevDay(const std::string& date_yyyy_mm_dd) const;
    std::string getNextDay(const std::string& date_yyyy_mm_dd) const;
    void renderUI();
    void setupInitialGeometry();
    void setWindowIcon();
    void saveWindowGeometry();
    ImVec4 getPriorityColor(Priority priority) const;
};

}

#endif
