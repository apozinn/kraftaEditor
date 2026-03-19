#!/usr/bin/env bash
set -euo pipefail

APP_NAME="Krafta Editor"
EXECUTABLE_NAME="kraftaEditor"
INSTALL_DIR="/usr/local/bin"
SHARE_DIR="/usr/share/kraftaEditor"
ICON_DEST="/usr/share/icons/hicolor/256x256/apps/kraftaEditor.png"
DESKTOP_FILE="/usr/share/applications/krafta-editor.desktop"

echo "Installing $APP_NAME..."

if [[ $EUID -ne 0 ]]; then
    echo "Error: please run as root (use sudo)."
    exit 1
fi

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# CMake generates bin/Release (capital R only), not bin/RELEASE
BUILD_PATH="$PROJECT_ROOT/build/bin/Release"

SOURCE_ASSETS="$PROJECT_ROOT/assets"
SOURCE_LANGUAGES="$PROJECT_ROOT/languages"
SOURCE_CONFIG="$PROJECT_ROOT/config"
SOURCE_I18N="$PROJECT_ROOT/i18n"
SOURCE_DESKTOP="$PROJECT_ROOT/resources/krafta-editor.desktop"

# --- Pre-flight checks -------------------------------------------------------

if [[ ! -f "$BUILD_PATH/$EXECUTABLE_NAME" ]]; then
    echo "Error: executable not found at $BUILD_PATH/$EXECUTABLE_NAME"
    echo "       Build the project first: cmake --build build --config Release"
    exit 1
fi

for dir in "$SOURCE_ASSETS" "$SOURCE_LANGUAGES" "$SOURCE_CONFIG" "$SOURCE_I18N"; do
    if [[ ! -d "$dir" ]]; then
        echo "Error: required directory not found: $dir"
        exit 1
    fi
done

if [[ ! -f "$SOURCE_DESKTOP" ]]; then
    echo "Error: desktop file not found at $SOURCE_DESKTOP"
    exit 1
fi

# --- Install -----------------------------------------------------------------

echo "  Installing executable → $INSTALL_DIR/$EXECUTABLE_NAME"
install -Dm755 "$BUILD_PATH/$EXECUTABLE_NAME" "$INSTALL_DIR/$EXECUTABLE_NAME"

echo "  Installing assets     → $SHARE_DIR/"
for dir in assets languages config i18n; do
    rm -rf "$SHARE_DIR/$dir"
    mkdir -p "$SHARE_DIR/$dir"
    cp -r "$PROJECT_ROOT/$dir/." "$SHARE_DIR/$dir/"
done

echo "  Installing icon       → $ICON_DEST"
install -Dm644 "$SOURCE_ASSETS/images/kraftaEditor.png" "$ICON_DEST"

echo "  Installing .desktop   → $DESKTOP_FILE"
install -Dm644 "$SOURCE_DESKTOP" "$DESKTOP_FILE"

# --- Refresh system caches ---------------------------------------------------

echo "  Updating desktop database..."
update-desktop-database /usr/share/applications/ 2>/dev/null || \
    echo "  Warning: update-desktop-database failed — app may not appear in menu"

echo "  Updating icon cache..."
gtk-update-icon-cache -f -t /usr/share/icons/hicolor/ 2>/dev/null || \
    echo "  Warning: gtk-update-icon-cache failed — icon may not appear correctly"

echo ""
echo "$APP_NAME installed successfully."
echo "Launch from the applications menu or run: $EXECUTABLE_NAME"