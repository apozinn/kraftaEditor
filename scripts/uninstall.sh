#!/usr/bin/env bash
set -euo pipefail

APP_NAME="Krafta Editor"
EXECUTABLE_NAME="kraftaEditor"
INSTALL_DIR="/usr/local/bin"
SHARE_DIR="/usr/share/kraftaEditor"
ICON_PATH="/usr/share/icons/hicolor/256x256/apps/kraftaEditor.png"
DESKTOP_FILE="/usr/share/applications/krafta-editor.desktop"

echo "Uninstalling $APP_NAME..."

if [[ $EUID -ne 0 ]]; then
    echo "Error: please run as root (use sudo)."
    exit 1
fi

# --- Remove files ------------------------------------------------------------

if [[ -f "$INSTALL_DIR/$EXECUTABLE_NAME" ]]; then
    echo "  Removing executable..."
    rm -f "$INSTALL_DIR/$EXECUTABLE_NAME"
else
    echo "  Executable not found, skipping."
fi

if [[ -d "$SHARE_DIR" ]]; then
    echo "  Removing shared data..."
    rm -rf "$SHARE_DIR"
else
    echo "  Shared data not found, skipping."
fi

if [[ -f "$ICON_PATH" ]]; then
    echo "  Removing icon..."
    rm -f "$ICON_PATH"
else
    echo "  Icon not found, skipping."
fi

if [[ -f "$DESKTOP_FILE" ]]; then
    echo "  Removing desktop entry..."
    rm -f "$DESKTOP_FILE"
else
    echo "  Desktop entry not found, skipping."
fi

# --- Refresh system caches ---------------------------------------------------

echo "  Updating desktop database..."
update-desktop-database /usr/share/applications/ 2>/dev/null || \
    echo "  Warning: update-desktop-database failed"

echo "  Updating icon cache..."
gtk-update-icon-cache -f -t /usr/share/icons/hicolor/ 2>/dev/null || \
    echo "  Warning: gtk-update-icon-cache failed"

echo ""
echo "$APP_NAME uninstalled successfully."