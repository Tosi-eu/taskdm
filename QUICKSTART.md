# Quick Start Guide

## Prerequisites

Install dependencies on Ubuntu:

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

## Setup

1. **Download external dependencies:**

```bash
./setup_dependencies.sh
```

Or manually:
```bash
cd external
git clone https://github.com/ocornut/imgui.git
cd ..
```

2. **Build the project:**

```bash
mkdir build
cd build
cmake ..
make
```

3. **Run CLI:**

```bash
./task-cli add "My first task" --priority high
./task-cli list
```

4. **Run GUI:**

```bash
./task-gui
```

## Executar ao iniciar o sistema (Autostart)

Para o Task Manager abrir automaticamente quando você fizer login:

```bash
chmod +x setup_autostart.sh
./setup_autostart.sh
```

Após o próximo login, a interface será exibida automaticamente.

Para desabilitar: remova `~/.config/autostart/task-dm.desktop`

## Alias task-dm

Após `make install`, você pode usar `task-dm` como atalho para `task-gui`:

```bash
sudo make install  
task-dm            
```

## Project Structure

```
taskdm/
├── CMakeLists.txt         
├── README.md              
├── QUICKSTART.md           
├── setup_dependencies.sh  
│
├── src/                   
│   ├── core/             
│   ├── database/         
│   ├── repository/       
│   ├── services/         
│   ├── cli/               
│   ├── gui/              
│   └── config/           
│
├── include/               
│
├── external/               
│   └── imgui/            
│
├── config/                
│   └── config.json       
│
└── db/                    
    └── tasks.db         
```

## Installable .deb package

To build a Debian/Ubuntu package and install it:

```bash
chmod +x build_deb.sh
./build_deb.sh
sudo dpkg -i taskdm_1.0.0_*.deb
```

After installation you can use `task-cli`, `task-gui`, and `task-dm` (alias for the GUI) from anywhere.

**Uninstall:**

```bash
sudo apt remove taskdm
```

Or with dpkg: `sudo dpkg -r taskdm`. This removes the binaries; your config (`~/.config/taskdm/`) and data (`~/.local/share/taskdm/`) are left in place. To remove those too: `rm -rf ~/.config/taskdm ~/.local/share/taskdm`. Disable autostart if you set it up: `rm ~/.config/autostart/task-dm.desktop`.

## Config command (widget position / monitor corner)

Customize where the widget appears on the monitor:

```bash
./task-cli config get
./task-cli config set position top-left
./task-cli config set position bottom-right
```

Positions: `top-left`, `top-right`, `bottom-left`, `bottom-right`. You can also set `widget_width` and `widget_height`. Config is saved to `~/.config/taskdm/config.json` when not running from the project directory.

## Common Commands

```bash
./task-cli add "Task title" --priority high

./task-cli list

./task-cli done <id>

./task-cli remove <id>

./task-cli priority <id> urgent
```

## Troubleshooting

- **CMake errors**: Ensure all system dependencies are installed
- **ImGui not found**: Run `./setup_dependencies.sh` or manually download to `external/imgui/`
- **Database errors**: Ensure write permissions in project directory
- **GUI not showing**: Check OpenGL drivers: `glxinfo | grep "OpenGL version"`
