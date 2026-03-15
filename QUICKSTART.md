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

## Toaster notifications (uncompleted tasks)

When there are active tasks, you get a reminder:

- **Wayland**: uses **system notifications** (`notify-send`). The **position is set by your compositor** (e.g. GNOME, KDE, Sway), often top-right — to change it, configure the desktop/notification daemon, not TaskDM. Requires `libnotify-bin` (e.g. `sudo apt install libnotify-bin`). A **notification sound** is played if `toaster_sound_enabled` is on (default); uses `canberra-gtk-play` or `paplay` (install `libcanberra-gtk3` or have PulseAudio + freedesktop sound theme).
- **X11**: a **separate toaster window** appears on the monitor (small popup with shake animation). Its position is controlled by `toaster_position` (default bottom-right).

Intervals are **shorter for higher priority** (URGENT more often, LOW less often). Configure in `config.json`:

- `toaster_enabled`: 1 or 0
- `toaster_first_delay_ms`: milliseconds before the **first** notification (default 30000 = 30 sec)
- `toaster_interval_urgent_ms`, `toaster_interval_high_ms`, `toaster_interval_medium_ms`, `toaster_interval_low_ms`: milliseconds between **later** reminders (defaults: 5min, 15min, 30min, 60min)
- `toaster_duration_ms`: how long the toaster/notification stays visible in ms (default 10000 = 10s)
- `toaster_position`: **only on X11** (toaster window). 0=top-left, **1=bottom-right (default)**, 2=top-right, 3=bottom-left. On Wayland the position is set by the compositor.
- `toaster_sound_enabled`: 1 or 0 — play a sound when the reminder appears (default 1)

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
