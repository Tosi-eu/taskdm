#!/bin/bash

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
cd "$SCRIPT_DIR"

VERSION="${TASKDM_VERSION:-1.0.0}"
ARCH="$(dpkg --print-architecture 2>/dev/null || echo amd64)"
PACKAGE_NAME="taskdm"
DEB_DIR="${PACKAGE_NAME}_${VERSION}_${ARCH}"
BUILD_DIR="build"

echo "=== Building Task Manager ==="
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc 2>/dev/null || echo 2)
cd ..

echo "=== Creating package structure ==="
rm -rf "$DEB_DIR"
mkdir -p "$DEB_DIR/usr/bin"
mkdir -p "$DEB_DIR/usr/share/pixmaps"
mkdir -p "$DEB_DIR/usr/share/applications"
mkdir -p "$DEB_DIR/DEBIAN"

install -m 0755 "$BUILD_DIR/task-cli" "$DEB_DIR/usr/bin/"
install -m 0755 "$BUILD_DIR/task-gui" "$DEB_DIR/usr/bin/"
if [ -f "$SCRIPT_DIR/public/logo.png" ]; then
    install -m 0644 "$SCRIPT_DIR/public/logo.png" "$DEB_DIR/usr/share/pixmaps/taskdm.png"
fi
if [ -f "$SCRIPT_DIR/public/check.png" ]; then
    install -m 0644 "$SCRIPT_DIR/public/check.png" "$DEB_DIR/usr/share/pixmaps/taskdm_check.png"
fi
if [ -f "$SCRIPT_DIR/public/error.png" ]; then
    install -m 0644 "$SCRIPT_DIR/public/error.png" "$DEB_DIR/usr/share/pixmaps/taskdm_error.png"
fi
cat > "$DEB_DIR/usr/share/applications/taskdm.desktop" << 'DESKTOP'
[Desktop Entry]
Type=Application
Name=Task Manager
Comment=Task Manager widget - your tasks on screen
Exec=task-gui
Icon=taskdm
Terminal=false
Categories=Utility;
StartupWMClass=TaskDM
DESKTOP

cat > "$DEB_DIR/DEBIAN/control" << EOF
Package: $PACKAGE_NAME
Version: $VERSION
Section: utils
Priority: optional
Architecture: $ARCH
Depends: libc6 (>= 2.31), libsqlite3-0, libglfw3 (>= 3.2), libgl1, libglu1-mesa
Maintainer: Task Manager <noreply@localhost>
Description: Lightweight desktop task manager with CLI and GUI
 Task Manager (taskdm) provides a small always-on-top widget and a CLI
 for managing tasks. Tasks are stored in SQLite.
 - task-cli: add, list, complete, remove tasks from terminal
 - task-gui / task-dm: floating widget on desktop
EOF

cat > "$DEB_DIR/DEBIAN/postinst" << 'POSTINST'
#!/bin/sh
set -e
if [ "$1" = "configure" ]; then
    if [ -x /usr/bin/task-gui ] && [ ! -e /usr/bin/task-dm ]; then
        ln -sf task-gui /usr/bin/task-dm
    fi
fi
POSTINST
chmod 755 "$DEB_DIR/DEBIAN/postinst"

cat > "$DEB_DIR/DEBIAN/postrm" << 'POSTRM'
#!/bin/sh
if [ "$1" = "purge" ] && [ -L /usr/bin/task-dm ]; then
    rm -f /usr/bin/task-dm
fi
POSTRM
chmod 755 "$DEB_DIR/DEBIAN/postrm"

echo "=== Building .deb ==="
dpkg-deb --build "$DEB_DIR"
DEB_FILE="${DEB_DIR}.deb"

echo ""
echo "Done. Package: $DEB_FILE"
echo "Install:   sudo dpkg -i $DEB_FILE"
echo "Uninstall: sudo apt remove taskdm"
echo "Fix deps:  sudo apt-get install -f"
echo "Config:    task-cli config get  |  task-cli config set position top-right|top-left|bottom-right|bottom-left|custom"
echo "Linux (Pop/Ubuntu/Wayland): Window uses native title bar so drag works. To force undecorated: env TASKDM_UNDECORATED=1 task-gui"