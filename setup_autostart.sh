#!/bin/bash

set -e

AUTOSTART_DIR="$HOME/.config/autostart"
CONFIG_DIR="$HOME/.config/taskdm"
DATA_DIR="$HOME/.local/share/taskdm"
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

mkdir -p "$AUTOSTART_DIR"
mkdir -p "$CONFIG_DIR"
mkdir -p "$DATA_DIR"

DESKTOP_SRC="$SCRIPT_DIR/task-dm.desktop"
DESKTOP_DST="$AUTOSTART_DIR/task-dm.desktop"

BUILD_BIN="$SCRIPT_DIR/build/task-gui"
INSTALLED_BIN="$(which task-gui 2>/dev/null || true)"

if [ -x "$BUILD_BIN" ]; then
    EXEC_PATH="$BUILD_BIN"
elif [ -n "$INSTALLED_BIN" ]; then
    EXEC_PATH="$INSTALLED_BIN"
else
    echo "Aviso: task-gui não encontrado no PATH nem em $BUILD_BIN"
    echo "Execute 'make' e 'make install' primeiro, ou rode este script do diretório do projeto."
    EXEC_PATH="task-gui"
fi

cat > "$DESKTOP_DST" << EOF
[Desktop Entry]
Type=Application
Name=Task Manager
Comment=Task Manager widget - suas tarefas na tela
Exec=$EXEC_PATH
Icon=taskdm
Terminal=false
Hidden=false
X-GNOME-Autostart-enabled=true
EOF

if [ ! -f "$CONFIG_DIR/config.json" ]; then
    if [ -f "$SCRIPT_DIR/config/config.json" ]; then
        cp "$SCRIPT_DIR/config/config.json" "$CONFIG_DIR/config.json"
        sed -i "s|\"db_path\": \"db/tasks.db\"|\"db_path\": \"$DATA_DIR/tasks.db\"|g" "$CONFIG_DIR/config.json"
    else
        cat > "$CONFIG_DIR/config.json" << EOF
{
  "widget_width": 740,
  "widget_height": 440,
  "refresh_interval_ms": 2000,
  "db_path": "$DATA_DIR/tasks.db"
}
EOF
    fi
fi

echo "Autostart configurado!"
echo "  Desktop: $DESKTOP_DST"
echo "  Config:  $CONFIG_DIR/config.json"
echo "  DB:      $DATA_DIR/tasks.db"
echo ""
echo "O Task Manager será iniciado na próxima vez que você fizer login."
echo "Para desabilitar: rm $DESKTOP_DST"