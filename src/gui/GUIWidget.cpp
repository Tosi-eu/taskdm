#include "gui/GUIWidget.h"
#include "core/Task.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <iostream>
#include <string>
#include <chrono>
#include <cstdio>
#include <ctime>
#include <sstream>
#include <cstdint>
#include <cstdlib>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#ifdef _WIN32
#include <windows.h>
#endif
#if defined(__APPLE__)
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

namespace {

GLFWmonitor* getCurrentMonitor(GLFWwindow* window) {
    int win_x, win_y, win_w, win_h;
    glfwGetWindowPos(window, &win_x, &win_y);
    glfwGetWindowSize(window, &win_w, &win_h);
    int best_area = 0;
    GLFWmonitor* best = nullptr;
    int count = 0;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    for (int i = 0; i < count; ++i) {
        GLFWmonitor* mon = monitors[i];
        int mx, my;
        glfwGetMonitorPos(mon, &mx, &my);
        const GLFWvidmode* mode = glfwGetVideoMode(mon);
        int mw = mode->width;
        int mh = mode->height;
        int overlap_x0 = (win_x > mx) ? win_x : mx;
        int overlap_y0 = (win_y > my) ? win_y : my;
        int overlap_x1 = (win_x + win_w < mx + mw) ? win_x + win_w : mx + mw;
        int overlap_y1 = (win_y + win_h < my + mh) ? win_y + win_h : my + mh;
        int area = 0;
        if (overlap_x1 > overlap_x0 && overlap_y1 > overlap_y0)
            area = (overlap_x1 - overlap_x0) * (overlap_y1 - overlap_y0);
        if (area > best_area) {
            best_area = area;
            best = mon;
        }
    }
    return best ? best : glfwGetPrimaryMonitor();
}

void clampPositionToMonitor(GLFWwindow* window, int w, int h, int* x, int* y) {
    GLFWmonitor* mon = window ? getCurrentMonitor(window) : glfwGetPrimaryMonitor();
    if (!mon) return;
    int mx, my;
    glfwGetMonitorPos(mon, &mx, &my);
    const GLFWvidmode* mode = glfwGetVideoMode(mon);
    int mw = mode->width;
    int mh = mode->height;
    if (*x < mx) *x = mx;
    if (*y < my) *y = my;
    if (*x + w > mx + mw) *x = mx + mw - w;
    if (*y + h > my + mh) *y = my + mh - h;
}

unsigned int loadTextureFromFile(const char* path) {
    int w = 0, h = 0, comp = 0;
    unsigned char* pixels = stbi_load(path, &w, &h, &comp, 4);
    if (!pixels || w <= 0 || h <= 0) return 0;
    GLuint tex = 0;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    glBindTexture(GL_TEXTURE_2D, 0);
    return static_cast<unsigned int>(tex);
}

}

namespace taskdm {

GUIWidget::GUIWidget(TaskService& service, const Config& config)
    : service_(service), config_(config), window_(nullptr), imgui_context_(nullptr) {
    last_refresh_ = std::chrono::steady_clock::now();
    last_config_check_ = std::chrono::steady_clock::now();
    history_date_ = getTodayDate();
    std::snprintf(history_from_buf_, sizeof(history_from_buf_), "%s", history_date_.c_str());
    std::snprintf(history_to_buf_, sizeof(history_to_buf_), "%s", history_date_.c_str());
}

GUIWidget::~GUIWidget() {
    shutdown();
}

bool GUIWidget::initialize() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(__linux__)
    use_native_title_bar_ = (std::getenv("TASKDM_UNDECORATED") == nullptr);
    glfwWindowHint(GLFW_DECORATED, use_native_title_bar_ ? GLFW_TRUE : GLFW_FALSE);
#else
    glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
#if GLFW_VERSION_MAJOR > 3 || (GLFW_VERSION_MAJOR == 3 && GLFW_VERSION_MINOR >= 4)
    glfwWindowHint(GLFW_ALWAYS_ON_TOP, GLFW_TRUE);
#endif
    glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    
#if defined(GLFW_X11_CLASS_NAME)
    glfwWindowHintString(GLFW_X11_CLASS_NAME, "TaskDM");
    glfwWindowHintString(GLFW_X11_INSTANCE_NAME, "TaskDM");
#endif
    window_ = glfwCreateWindow(
        config_.widget_width,
        config_.widget_height,
        "TaskDM",
        nullptr,
        nullptr
    );
    
    if (!window_) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window_);
    glfwSwapInterval(1);
    
    setupInitialGeometry();
    setWindowIcon();

    namespace fs = std::filesystem;
    fs::path cwd = fs::current_path();
    fs::path cwd_parent = cwd.parent_path();
    std::string check_paths[] = {
        "public/check.png", "../public/check.png",
        (cwd / "public" / "check.png").string(),
        (cwd_parent / "public" / "check.png").string(),
        "/usr/share/pixmaps/taskdm_check.png"
    };
    std::string error_paths[] = {
        "public/error.png", "../public/error.png",
        (cwd / "public" / "error.png").string(),
        (cwd_parent / "public" / "error.png").string(),
        "/usr/share/pixmaps/taskdm_error.png"
    };
    for (const auto& p : check_paths) { check_tex_ = loadTextureFromFile(p.c_str()); if (check_tex_) break; }
    for (const auto& p : error_paths) { error_tex_ = loadTextureFromFile(p.c_str()); if (error_tex_) break; }

    IMGUI_CHECKVERSION();
    imgui_context_ = ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    ImGui::StyleColorsDark();
    
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 330");
    
    updateTasks();
    
    return true;
}

void GUIWidget::setupInitialGeometry() {
    if (!window_) return;
    if (config_.window_x >= 0 && config_.window_y >= 0) {
        int x = config_.window_x;
        int y = config_.window_y;
        clampPositionToMonitor(window_, config_.widget_width, config_.widget_height, &x, &y);
        glfwSetWindowPos(window_, x, y);
    }
}

void GUIWidget::saveWindowGeometry() {
    if (!window_) return;
    int x, y, w, h;
    glfwGetWindowPos(window_, &x, &y);
    glfwGetWindowSize(window_, &w, &h);
    if (x >= 0 && y >= 0 && w > 0 && h > 0) {
        config_.window_x = x;
        config_.window_y = y;
        config_.widget_width = w;
        config_.widget_height = h;
        config_.saveToFile(Config::getDefaultConfigPath());
    }
}

void GUIWidget::setWindowIcon() {
    if (!window_) return;
    const char* paths[] = {
        "/usr/share/pixmaps/taskdm.png",
        "public/logo.png",
        "../public/logo.png",
        nullptr
    };
    for (const char** p = paths; *p; ++p) {
        int w = 0, h = 0, comp = 0;
        unsigned char* pixels = stbi_load(*p, &w, &h, &comp, 4);
        if (!pixels || w <= 0 || h <= 0) continue;
        GLFWimage img;
        img.width = w;
        img.height = h;
        img.pixels = pixels;
        glfwSetWindowIcon(window_, 1, &img);
        stbi_image_free(pixels);
        return;
    }
}

void GUIWidget::updateTasks() {
    current_tasks_ = service_.listTasks(false);
    finished_tasks_ = service_.listCompletedTasks();
    last_refresh_ = std::chrono::steady_clock::now();
}

void GUIWidget::updateHistoryTasks() {
    history_tasks_ = service_.listCompletedTasksByDate(history_date_);
}

void GUIWidget::updateHistoryTasksRange() {
    std::string from_str(history_from_buf_);
    std::string to_str(history_to_buf_);
    if (from_str.empty() || to_str.empty()) return;
    if (from_str > to_str) std::swap(from_str, to_str);
    history_tasks_ = service_.listCompletedTasksByDateRange(from_str, to_str);
}

std::string GUIWidget::getTodayDate() const {
    std::time_t now = std::time(nullptr);
    std::tm* tm = std::localtime(&now);
    if (!tm) return "0000-00-00";
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday);
    return std::string(buf);
}

std::string GUIWidget::getPrevDay(const std::string& date_yyyy_mm_dd) const {
    int y, m, d;
    if (std::sscanf(date_yyyy_mm_dd.c_str(), "%d-%d-%d", &y, &m, &d) != 3) return date_yyyy_mm_dd;
    std::tm t = {};
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = d;
    t.tm_isdst = -1;
    std::time_t sec = std::mktime(&t);
    if (sec == static_cast<std::time_t>(-1)) return date_yyyy_mm_dd;
    sec -= 86400;
    std::tm* next = std::localtime(&sec);
    if (!next) return date_yyyy_mm_dd;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", next->tm_year + 1900, next->tm_mon + 1, next->tm_mday);
    return std::string(buf);
}

std::string GUIWidget::getNextDay(const std::string& date_yyyy_mm_dd) const {
    int y, m, d;
    if (std::sscanf(date_yyyy_mm_dd.c_str(), "%d-%d-%d", &y, &m, &d) != 3) return date_yyyy_mm_dd;
    std::tm t = {};
    t.tm_year = y - 1900;
    t.tm_mon = m - 1;
    t.tm_mday = d;
    t.tm_isdst = -1;
    std::time_t sec = std::mktime(&t);
    if (sec == static_cast<std::time_t>(-1)) return date_yyyy_mm_dd;
    sec += 86400;
    std::tm* next = std::localtime(&sec);
    if (!next) return date_yyyy_mm_dd;
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%04d-%02d-%02d", next->tm_year + 1900, next->tm_mon + 1, next->tm_mday);
    return std::string(buf);
}

ImVec4 GUIWidget::getPriorityColor(Priority priority) const {
    switch (priority) {
        case Priority::LOW:
            return ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
        case Priority::MEDIUM:
            return ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
        case Priority::HIGH:
            return ImVec4(1.0f, 0.5f, 0.0f, 1.0f);
        case Priority::URGENT:
            return ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
        default:
            return ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
    }
}

void GUIWidget::renderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_refresh_).count();
    if (elapsed >= config_.refresh_interval_ms) {
        updateTasks();
    }
    auto config_elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_config_check_).count();
    if (config_elapsed >= 1500) {
        last_config_check_ = now;
        int x, y, w, h;
        glfwGetWindowPos(window_, &x, &y);
        glfwGetWindowSize(window_, &w, &h);
        if (x >= 0 && y >= 0 && w > 0 && h > 0 &&
            (x != config_.window_x || y != config_.window_y || w != config_.widget_width || h != config_.widget_height)) {
            config_.window_x = x;
            config_.window_y = y;
            config_.widget_width = w;
            config_.widget_height = h;
            config_.saveToFile(Config::getDefaultConfigPath());
        }
        Config fresh;
        std::string path = Config::getDefaultConfigPath();
        if (fresh.loadFromFile(path)) {
            int fw, fh;
            glfwGetWindowSize(window_, &fw, &fh);
            bool size_changed = (fresh.widget_width != fw || fresh.widget_height != fh);
            if (size_changed && fresh.widget_width > 0 && fresh.widget_height > 0) {
                config_.widget_width = fresh.widget_width;
                config_.widget_height = fresh.widget_height;
                glfwSetWindowSize(window_, config_.widget_width, config_.widget_height);
            }
            config_.refresh_interval_ms = fresh.refresh_interval_ms;
            config_.db_path = fresh.db_path;
        }
    }

    if (!use_native_title_bar_) {
        int win_w, win_h;
        glfwGetWindowSize(window_, &win_w, &win_h);
        const int title_bar_px = 24;
        const int drag_right_margin_px = 2 * 28 + 16;
        double cursor_x, cursor_y;
        glfwGetCursorPos(window_, &cursor_x, &cursor_y);
        bool in_title_bar = (cursor_y >= 0 && cursor_y < title_bar_px &&
                             cursor_x >= 0 && cursor_x < (win_w - drag_right_margin_px));
        int mouse_left = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT);
        bool mouse_down = (mouse_left == GLFW_PRESS);
        bool clicked = !mouse_left_down_prev_ && mouse_down;

        if (in_title_bar && clicked) {
            drag_state_.dragging = true;
            glfwGetWindowPos(window_, &drag_state_.start_win_x, &drag_state_.start_win_y);
            drag_state_.offset_x = cursor_x;
            drag_state_.offset_y = cursor_y;
        }
        if (drag_state_.dragging) {
            double mouse_x, mouse_y;
            glfwGetCursorPos(window_, &mouse_x, &mouse_y);
            int new_x = drag_state_.start_win_x + static_cast<int>(mouse_x - drag_state_.offset_x);
            int new_y = drag_state_.start_win_y + static_cast<int>(mouse_y - drag_state_.offset_y);
            glfwSetWindowPos(window_, new_x, new_y);
            glfwPollEvents();
            glfwGetWindowPos(window_, &drag_state_.start_win_x, &drag_state_.start_win_y);
        }
        if (!mouse_down) {
            if (drag_state_.dragging) {
                drag_state_.dragging = false;
                saveWindowGeometry();
            }
        }
        mouse_left_down_prev_ = mouse_down;
    }

    int fb_w, fb_h;
    glfwGetFramebufferSize(window_, &fb_w, &fb_h);
    ImGui::SetNextWindowSize(ImVec2(static_cast<float>(fb_w), static_cast<float>(fb_h)), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Once);
    
    ImGui::Begin("Tasks", nullptr, 
                 ImGuiWindowFlags_NoTitleBar | 
                 ImGuiWindowFlags_NoResize | 
                 ImGuiWindowFlags_NoMove |
                 ImGuiWindowFlags_NoCollapse);
    
    const float title_bar_h = 24.0f;
    if (!use_native_title_bar_) {
        const float btn_w = 28.0f;
        const float btn_h = title_bar_h - 4.0f;
        const float right_offset = 2.0f * btn_w + ImGui::GetStyle().ItemSpacing.x + 8.0f;
        ImGui::SetCursorPos(ImVec2(8.0f, (title_bar_h - ImGui::GetTextLineHeight()) * 0.5f));
        ImGui::Text("Tasks");
        ImGui::SameLine(ImGui::GetWindowWidth() - right_offset);
        ImGui::SetCursorPosY(2.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.35f, 0.35f, 0.35f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
        if (ImGui::Button("−", ImVec2(btn_w, btn_h))) {
            if (window_)
                glfwIconifyWindow(window_);
        }
        ImGui::SameLine();
        if (ImGui::Button("×", ImVec2(btn_w, btn_h))) {
            if (window_)
                glfwSetWindowShouldClose(window_, GLFW_TRUE);
        }
        ImGui::PopStyleColor(3);
        ImGui::SetCursorPos(ImVec2(0, title_bar_h));
    }
    ImGui::Separator();
    if (ImGui::BeginTabBar("##TaskTabs", ImGuiTabBarFlags_None)) {
        if (ImGui::BeginTabItem("Active")) {
            ImGui::Text("Active Tasks");
            const float add_btn_w = 24.0f;
            ImGui::SameLine(ImGui::GetWindowWidth() - add_btn_w - 8.0f);
            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.4f, 0.2f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.5f, 0.3f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.25f, 0.45f, 0.25f, 1.0f));
            if (ImGui::Button("+", ImVec2(add_btn_w, add_btn_w))) {
                show_add_form_ = true;
                new_task_buf_[0] = '\0';
                new_task_priority_idx_ = 1;
            }
            ImGui::PopStyleColor(3);
            ImGui::Separator();
            if (show_add_form_) {
                ImGui::PushItemWidth(-1);
                ImGui::InputTextWithHint("##newtitle", "Task title...", new_task_buf_, sizeof(new_task_buf_));
                const char* priorities[] = { "LOW", "MEDIUM", "HIGH", "URGENT" };
                ImGui::Combo("Priority", &new_task_priority_idx_, priorities, 4);
                ImGui::PopItemWidth();
                if (ImGui::Button("Add")) {
                    std::string title(new_task_buf_);
                    if (!title.empty()) {
                        Priority p = static_cast<Priority>(new_task_priority_idx_);
                        if (service_.addTask(title, p)) {
                            updateTasks();
                            show_add_form_ = false;
                            new_task_buf_[0] = '\0';
                        }
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    show_add_form_ = false;
                    new_task_buf_[0] = '\0';
                }
                ImGui::Separator();
            }
            if (current_tasks_.empty() && !show_add_form_) {
                ImGui::Text("No active tasks");
            }
            if (!current_tasks_.empty()) {
                for (size_t i = 0; i < current_tasks_.size(); ++i) {
                    Task& task = current_tasks_[i];
                    ImGui::PushID(static_cast<int>(task.getId()));
                    ImVec4 color = getPriorityColor(task.getPriority());
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    if (error_tex_) {
                        ImGui::Image((ImTextureID)(uintptr_t)error_tex_, ImVec2(16, 16));
                        ImGui::SameLine();
                    }
                    bool completed = task.isCompleted();
                    if (ImGui::Checkbox("", &completed)) {
                        if (completed) {
                            service_.completeTask(task.getId());
                            updateTasks();
                            ImGui::PopStyleColor();
                            ImGui::PopID();
                            break;
                        }
                    }
                    ImGui::SameLine();
                    ImGui::Text("%s", task.getTitle().c_str());
                    ImGui::SameLine();
                    if (ImGui::SmallButton("x")) {
                        service_.removeTask(task.getId());
                        updateTasks();
                        ImGui::PopStyleColor();
                        ImGui::PopID();
                        break;
                    }
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                        int id = task.getId();
                        ImGui::SetDragDropPayload("TASK_ROW", &id, sizeof(int));
                        ImGui::Text("Move \"%s\"", task.getTitle().c_str());
                        ImGui::EndDragDropSource();
                    }
                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TASK_ROW")) {
                            if (payload->DataSize == sizeof(int)) {
                                int dragged_id = *static_cast<const int*>(payload->Data);
                                if (service_.reorderTask(dragged_id, static_cast<int>(i))) updateTasks();
                            }
                        }
                        ImGui::EndDragDropTarget();
                    }
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                }
            }
            if (show_add_form_ && current_tasks_.empty()) {
                ImGui::Text("(add your task above)");
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Finished")) {
            ImGui::Text("Closed / Completed");
            ImGui::Separator();
            if (finished_tasks_.empty()) {
                ImGui::Text("No finished tasks yet");
            } else {
                for (const Task& task : finished_tasks_) {
                    ImGui::PushID(task.getId());
                    if (check_tex_) {
                        ImGui::Image((ImTextureID)(uintptr_t)check_tex_, ImVec2(16, 16));
                        ImGui::SameLine();
                    }
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.6f, 0.6f, 0.6f, 1.0f));
                    ImGui::TextUnformatted(task.getTitle().c_str());
                    ImGui::PopStyleColor();
                    ImGui::SameLine();
                    if (ImGui::SmallButton("x")) {
                        service_.removeTask(task.getId());
                        updateTasks();
                        ImGui::PopID();
                        break;
                    }
                    ImGui::PopID();
                }
            }
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("History")) {
            ImGui::Text("Completed tasks");
            ImGui::Separator();
            if (history_date_.empty()) history_date_ = getTodayDate();
            ImGui::Checkbox("Date range (from / to)", &history_use_range_);
            if (history_use_range_) {
                ImGui::SetNextItemWidth(120);
                ImGui::InputTextWithHint("##from", "From (YYYY-MM-DD)", history_from_buf_, sizeof(history_from_buf_));
                ImGui::SameLine();
                ImGui::SetNextItemWidth(120);
                ImGui::InputTextWithHint("##to", "To (YYYY-MM-DD)", history_to_buf_, sizeof(history_to_buf_));
                ImGui::SameLine();
                if (ImGui::Button("Search")) {
                    updateHistoryTasksRange();
                }
            } else {
                if (ImGui::Button("< Prev")) {
                    history_date_ = getPrevDay(history_date_);
                }
                ImGui::SameLine();
                ImGui::Text("%s", history_date_.c_str());
                ImGui::SameLine();
                if (ImGui::Button("Next >")) {
                    history_date_ = getNextDay(history_date_);
                }
                if (history_date_ == getTodayDate()) {
                    ImGui::SameLine();
                    ImGui::TextDisabled("(today)");
                }
                updateHistoryTasks();
            }
            ImGui::Separator();
            if (history_tasks_.empty()) {
                ImGui::Text("%s", history_use_range_ ? "No tasks in this range" : "No tasks completed on this day");
            } else {
                for (const Task& task : history_tasks_) {
                    ImGui::PushID(task.getId());
                    if (check_tex_) {
                        ImGui::Image((ImTextureID)(uintptr_t)check_tex_, ImVec2(16, 16));
                        ImGui::SameLine();
                    }
                    ImVec4 color = getPriorityColor(task.getPriority());
                    ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::TextUnformatted(task.getTitle().c_str());
                    ImGui::PopStyleColor();
                    ImGui::PopID();
                }
            }
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
    
    ImGui::Render();
    
    int display_w, display_h;
    glfwGetFramebufferSize(window_, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
    glfwSwapBuffers(window_);
}

void GUIWidget::run() {
    if (!window_) {
        return;
    }
    
    while (!glfwWindowShouldClose(window_)) {
        glfwPollEvents();
        renderUI();
    }
}

void GUIWidget::shutdown() {
    if (window_) {
        glfwMakeContextCurrent(window_);
        saveWindowGeometry();
        if (check_tex_) {
            GLuint t = static_cast<GLuint>(check_tex_);
            glDeleteTextures(1, &t);
            check_tex_ = 0;
        }
        if (error_tex_) {
            GLuint t = static_cast<GLuint>(error_tex_);
            glDeleteTextures(1, &t);
            error_tex_ = 0;
        }
    }
    if (imgui_context_) {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(imgui_context_);
        imgui_context_ = nullptr;
    }
    
    if (window_) {
        glfwDestroyWindow(window_);
        window_ = nullptr;
    }
    
    glfwTerminate();
}

}
