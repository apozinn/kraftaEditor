#!/usr/bin/env bash
set -e

APP_NAME="Krafta Editor"
EXECUTABLE_NAME="kraftaEditor"
INSTALL_DIR="/usr/local/bin"
SHARE_DIR="/usr/share/kraftaEditor"
DESKTOP_FILE="/usr/share/applications/kraftaEditor.desktop"

echo "Uninstalling $APP_NAME..."

if [[ $EUID -ne 0 ]]; then
    echo "Please run as root (use sudo)."
    exit 1
fi

if [[ -f "$INSTALL_DIR/$EXECUTABLE_NAME" ]]; then
    echo "Removing executable..."
    rm -f "$INSTALL_DIR/$EXECUTABLE_NAME"
fi

if [[ -d "$SHARE_DIR" ]]; then
    echo "Removing shared data..."
    rm -rf "$SHARE_DIR"
fi

if [[ -f "$DESKTOP_FILE" ]]; then
    echo "Removing desktop entry..."
    rm -f "$DESKTOP_FILE"
fi

update-desktop-database >/dev/null 2>&1 || true

echo "$APP_NAME successfully uninstalled."
