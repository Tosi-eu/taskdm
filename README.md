# Task Manager (taskdm)

A lightweight desktop task management tool with both CLI and GUI interfaces, built with modern C++20.

## Overview

Task Manager is a simple, fast task management application designed for Linux (especially Ubuntu). It provides:

- **CLI Interface**: Primary interface for managing tasks from the terminal
- **GUI Widget**: Small floating widget that displays active tasks on your desktop
- **SQLite Persistence**: All tasks are stored in a local SQLite database
- **Priority Management**: Four priority levels (LOW, MEDIUM, HIGH, URGENT) with color coding

## Features

- ✅ Add, list, complete, and remove tasks via CLI
- ✅ Update task priorities
- ✅ Visual priority indication in GUI (color-coded)
- ✅ Small, always-on-top desktop widget
- ✅ Automatic database refresh
- ✅ Configurable widget position and behavior
- ✅ Fast startup and low memory footprint

## Architecture

The project follows a clean, modular architecture:

```
taskdm/
├── core/          - Task entity and domain logic
├── database/      - SQLite connection and schema management
├── repository/    - Data access layer (TaskRepository)
├── services/      - Business logic layer (TaskService)
├── cli/           - CLI command interface
├── gui/           - GUI widget implementation
└── config/        - Configuration management
```

### Design Principles

- **Object-Oriented Programming**: Clear separation of concerns with well-defined classes
- **RAII**: Resource management through constructors/destructors
- **Const Correctness**: Proper use of const where applicable
- **Smart Pointers**: Used where appropriate for memory management
- **No Global State**: All state is managed through objects

## Dependencies

### Required for Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y \
    build-essential \
    cmake \
    libsqlite3-dev \
    libglfw3-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev
```

### External Libraries

The following libraries need to be placed in the `external/` directory:

1. **Dear ImGui** (v1.89+)
   - Download from: https://github.com/ocornut/imgui
   - Extract to: `external/imgui/`
   - Required files: `imgui.cpp`, `imgui.h`, `imgui_draw.cpp`, `imgui_tables.cpp`, `imgui_widgets.cpp`, `backends/imgui_impl_glfw.cpp`, `backends/imgui_impl_glfw.h`, `backends/imgui_impl_opengl3.cpp`, `backends/imgui_impl_opengl3.h`

2. **CLI11** (optional, for enhanced CLI parsing)
   - Download from: https://github.com/CLIUtils/CLI11
   - Extract to: `external/cli11/`
   - Note: The current implementation uses a simple built-in parser, but CLI11 can be integrated for more advanced features

## Build Instructions

### 1. Clone/Download External Dependencies

```bash
# Download Dear ImGui
cd external
git clone https://github.com/ocornut/imgui.git
cd ..
```

### 2. Build the Project

```bash
mkdir build
cd build
cmake ..
make
```

### 3. Install (Optional)

```bash
sudo make install
```

This will install:
- `task-cli` to `/usr/local/bin/`
- `task-gui` to `/usr/local/bin/`

## Running the Application

### CLI Interface

The CLI is the primary interface for managing tasks:

```bash
# From build directory
./task-cli add "Read AWS documentation" --priority high
./task-cli add "Fix bug in service" --priority urgent
./task-cli list
./task-cli done 2
./task-cli remove 1
./task-cli priority 2 medium
```

Or if installed:

```bash
task-cli add "Task title" --priority high
task-cli list
```

### GUI Widget

Run the GUI widget:

```bash
# From build directory
./task-gui
```

Or if installed:

```bash
task-gui
```

The widget will appear as a small, borderless window positioned according to your configuration (default: top-right corner).

## CLI Commands

### Add Task

```bash
task-cli add "Task title" [--priority <level>]
```

Adds a new task with optional priority. Priority defaults to MEDIUM if not specified.

**Priority levels**: `LOW`, `MEDIUM`, `HIGH`, `URGENT`

**Example**:
```bash
task-cli add "Review pull request" --priority high
```

### List Tasks

```bash
task-cli list
```

Displays all active (non-completed) tasks with their ID, status, priority, and title.

### Complete Task

```bash
task-cli done <id>
```

Marks a task as completed by its ID.

**Example**:
```bash
task-cli done 3
```

### Remove Task

```bash
task-cli remove <id>
```

Permanently removes a task from the database.

**Example**:
```bash
task-cli remove 2
```

### Update Priority

```bash
task-cli priority <id> <level>
```

Updates the priority of an existing task.

**Example**:
```bash
task-cli priority 1 urgent
```

## Configuration

Configuration is stored in `config/config.json`:

```json
{
  "widget_width": 740,
  "widget_height": 440,
  "window_x": -1,
  "window_y": -1,
  "refresh_interval_ms": 2000,
  "db_path": "db/tasks.db"
}
```

### Configuration Options

- **widget_width**: Initial width of the window in pixels (default: 740). The window is resizable.

- **widget_height**: Initial height of the window in pixels (default: 440). The window is resizable.

- **window_x**, **window_y**: Last window position (set automatically when you move/resize the window)

- **refresh_interval_ms**: How often the GUI refreshes tasks from the database in milliseconds (default: 2000)

- **db_path**: Path to the SQLite database file (default: `"db/tasks.db"`)

## Database Schema

Tasks are stored in a SQLite database with the following schema:

```sql
CREATE TABLE tasks (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    title TEXT NOT NULL,
    priority INTEGER NOT NULL,
    completed INTEGER NOT NULL DEFAULT 0,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);
```

The database file is created automatically on first run in the location specified by `db_path` in the configuration.

## Priority Colors

Tasks are color-coded in the GUI based on priority:

- **LOW** → Gray
- **MEDIUM** → Yellow
- **HIGH** → Orange
- **URGENT** → Red

## Development

### Project Structure

```
taskdm/
│
├── CMakeLists.txt          - CMake build configuration
├── README.md               - This file
│
├── src/                    - Source files
│   ├── core/              - Task implementation
│   ├── database/          - Database connection
│   ├── repository/        - Data access layer
│   ├── services/          - Business logic
│   ├── cli/               - CLI interface
│   ├── gui/               - GUI widget
│   └── config/            - Configuration handling
│
├── include/                - Header files (mirrors src structure)
│
├── external/               - External dependencies
│   └── imgui/             - Dear ImGui library
│
├── config/                 - Configuration files
│   └── config.json        - Default configuration
│
└── db/                     - Database storage
    └── tasks.db           - SQLite database (created automatically)
```

### Code Style

- Modern C++20 features
- RAII for resource management
- Const correctness
- Meaningful class and function names
- Separation of headers and implementation
- Comments where necessary for clarity

## Troubleshooting

### Database Errors

If you encounter database errors, ensure:
- The `db/` directory exists or can be created
- You have write permissions in the project directory

### GUI Not Displaying

If the GUI doesn't appear:
- Check that OpenGL drivers are installed: `glxinfo | grep "OpenGL version"`
- Verify GLFW is properly installed
- Check that Dear ImGui is correctly placed in `external/imgui/`

### Build Errors

If CMake fails:
- Ensure all dependencies are installed
- Verify Dear ImGui is in `external/imgui/`
- Check that CMake version is 3.15 or higher

## License

This project is provided as-is for educational and personal use.

## Contributing

This is a clean, well-structured project skeleton. Feel free to extend it with additional features such as:
- Task categories/tags
- Due dates
- Task descriptions
- Search and filtering
- Export/import functionality
- Multiple database support
