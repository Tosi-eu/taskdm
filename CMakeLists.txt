cmake_minimum_required(VERSION 3.15)
project(TaskManager VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_GUI "Build GUI widget" ON)
option(BUILD_CLI "Build CLI interface" ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/external)

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(SQLite3 REQUIRED)

set(IMGUI_DIR ${CMAKE_SOURCE_DIR}/external/imgui)
if(NOT EXISTS ${IMGUI_DIR})
    message(STATUS "Dear ImGui not found. Please download it to external/imgui")
endif()

set(CORE_SOURCES
    src/core/Task.cpp
)

set(DATABASE_SOURCES
    src/database/Database.cpp
)

set(REPOSITORY_SOURCES
    src/repository/TaskRepository.cpp
)

set(SERVICE_SOURCES
    src/services/TaskService.cpp
)

set(CLI_SOURCES
    src/cli/CLIInterface.cpp
    src/cli/main_cli.cpp
)

set(GUI_SOURCES
    src/gui/GUIWidget.cpp
    src/gui/main_gui.cpp
)

set(CONFIG_SOURCES
    src/config/Config.cpp
)

set(COMMON_SOURCES
    ${CORE_SOURCES}
    ${DATABASE_SOURCES}
    ${REPOSITORY_SOURCES}
    ${SERVICE_SOURCES}
    ${CONFIG_SOURCES}
)

if(BUILD_CLI)
    add_executable(task-cli
        ${CLI_SOURCES}
        ${COMMON_SOURCES}
    )
    
    target_link_libraries(task-cli
        SQLite::SQLite3
    )
endif()

if(BUILD_GUI)
    if(EXISTS ${IMGUI_DIR})
        set(IMGUI_SOURCES
            "${IMGUI_DIR}/imgui.cpp"
            "${IMGUI_DIR}/imgui_draw.cpp"
            "${IMGUI_DIR}/imgui_tables.cpp"
            "${IMGUI_DIR}/imgui_widgets.cpp"
            "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
            "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
        )
    else()
        message(WARNING "Dear ImGui not found at ${IMGUI_DIR}. GUI will not build.")
        set(BUILD_GUI OFF)
    endif()
    
    if(BUILD_GUI)
        add_executable(task-gui
            ${GUI_SOURCES}
            ${COMMON_SOURCES}
            ${IMGUI_SOURCES}
        )
        
        target_link_libraries(task-gui
            OpenGL::GL
            glfw
            SQLite::SQLite3
        )
        
        target_include_directories(task-gui PRIVATE 
            ${IMGUI_DIR}
            ${IMGUI_DIR}/backends
        )
    endif()
endif()

install(TARGETS task-cli task-gui
    RUNTIME DESTINATION bin
)

install(CODE "
  file(CREATE_LINK \"task-gui\" \"\$ENV{DESTDIR}\${CMAKE_INSTALL_PREFIX}/bin/task-dm\" SYMBOLIC)
")
