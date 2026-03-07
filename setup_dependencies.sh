#!/bin/bash

# Setup script for Task Manager dependencies
# This script downloads and sets up external dependencies

set -e

EXTERNAL_DIR="external"
IMGUI_DIR="${EXTERNAL_DIR}/imgui"

echo "Setting up Task Manager dependencies..."

# Create external directory if it doesn't exist
mkdir -p "${EXTERNAL_DIR}"

# Download Dear ImGui if not present
if [ ! -d "${IMGUI_DIR}" ]; then
    echo "Downloading Dear ImGui..."
    cd "${EXTERNAL_DIR}"
    git clone --depth 1 --branch docking https://github.com/ocornut/imgui.git || \
    git clone --depth 1 https://github.com/ocornut/imgui.git
    cd ..
    echo "Dear ImGui downloaded successfully"
else
    echo "Dear ImGui already present"
fi

echo ""
echo "Dependencies setup complete!"
echo ""
echo "Next steps:"
echo "  1. Install system dependencies (see README.md)"
echo "  2. mkdir build && cd build"
echo "  3. cmake .."
echo "  4. make"
