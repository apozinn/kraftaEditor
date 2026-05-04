#!/usr/bin/env bash
set -euo pipefail

APP_NAME="Krafta Editor"
EXECUTABLE_NAME="kraftaEditor"
INSTALL_DIR="/usr/bin"
SHARE_DIR="/usr/share/kraftaEditor"
ICON_DEST="/usr/share/icons/hicolor/256x256/apps/kraftaEditor.png"
DESKTOP_FILE="/usr/share/applications/krafta-editor.desktop"

echo "Installing $APP_NAME..."

# --- Root check --------------------------------------------------------------
if [[ $EUID -ne 0 ]]; then
    echo "Error: please run as root (use sudo)."
    exit 1
fi

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"

# --- Locate the built executable (handles Release / RELEASE / release) -------
BUILD_PATH=""
for try_dir in Release RELEASE release RelWithDebInfo RELWITHDEBINFO; do
    candidate="$PROJECT_ROOT/build/bin/$try_dir/$EXECUTABLE_NAME"
    if [[ -f "$candidate" ]]; then
        BUILD_PATH="$PROJECT_ROOT/build/bin/$try_dir"
        echo "  Found build at: $BUILD_PATH"
        break
    fi
done

# Fallback: flat bin/ directory (some generators output directly there)
if [[ -z "$BUILD_PATH" && -f "$PROJECT_ROOT/build/bin/$EXECUTABLE_NAME" ]]; then
    BUILD_PATH="$PROJECT_ROOT/build/bin"
    echo "  Found build at: $BUILD_PATH (flat)"
fi

if [[ -z "$BUILD_PATH" ]]; then
    echo "Error: executable '$EXECUTABLE_NAME' not found under build/bin/."
    echo "       Tried: Release, RELEASE, release, RelWithDebInfo, RELWITHDEBINFO, and flat bin/."
    echo "       Build the project first:"
    echo "         cmake -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr"
    echo "         cmake --build build --config Release"
    exit 1
fi

# --- Required asset directories ----------------------------------------------
for dir in assets languages config i18n; do
    if [[ ! -d "$PROJECT_ROOT/$dir" ]]; then
        echo "Error: required directory not found: $PROJECT_ROOT/$dir"
        exit 1
    fi
done

# --- Required desktop file ---------------------------------------------------
SOURCE_DESKTOP="$PROJECT_ROOT/resources/krafta-editor.desktop"
if [[ ! -f "$SOURCE_DESKTOP" ]]; then
    echo "Error: desktop file not found at $SOURCE_DESKTOP"
    exit 1
fi

# --- Required icon -----------------------------------------------------------
SOURCE_ICON="$PROJECT_ROOT/assets/images/kraftaEditor.png"
if [[ ! -f "$SOURCE_ICON" ]]; then
    echo "Error: icon not found at $SOURCE_ICON"
    exit 1
fi

# --- Install executable ------------------------------------------------------
echo "  Installing executable → $INSTALL_DIR/$EXECUTABLE_NAME"
install -Dm755 "$BUILD_PATH/$EXECUTABLE_NAME" "$INSTALL_DIR/$EXECUTABLE_NAME"

# --- Install assets ----------------------------------------------------------
echo "  Installing assets     → $SHARE_DIR/"
for dir in assets languages config i18n; do
    rm -rf "$SHARE_DIR/$dir"
    mkdir -p "$SHARE_DIR/$dir"
    cp -r "$PROJECT_ROOT/$dir/." "$SHARE_DIR/$dir/"
done

# --- Install icon ------------------------------------------------------------
echo "  Installing icon       → $ICON_DEST"
install -Dm644 "$SOURCE_ICON" "$ICON_DEST"

# --- Install .desktop file ---------------------------------------------------
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