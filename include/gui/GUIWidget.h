#ifndef GUI_WIDGET_H
#define GUI_WIDGET_H

#include "services/TaskService.h"
#include "config/Config.h"
#include "core/Task.h"
#include <GLFW/glfw3.h>
#include <vector>
#include <chrono>

// Forward declarations
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
    ImGuiContext* imgui_context_;
    
    std::vector<Task> current_tasks_;
    std::vector<Task> finished_tasks_;
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
    int new_task_priority_idx_ = 1;  // 0=LOW, 1=MEDIUM, 2=HIGH, 3=URGENT
    unsigned int check_tex_ = 0;
    unsigned int error_tex_ = 0;
    bool use_native_title_bar_ = false;  // true on Linux: compositor handles drag (Wayland/X11)

    void updateTasks();
    void renderUI();
    void setupInitialGeometry();
    void setWindowIcon();
    void saveWindowGeometry();
    ImVec4 getPriorityColor(Priority priority) const;
};

} // namespace taskdm

#endif // GUI_WIDGET_H
