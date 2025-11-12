#!/usr/bin/env bash
set -e

APP_NAME="Krafta Editor"
EXECUTABLE_NAME="kraftaEditor"
INSTALL_DIR="/usr/local/bin"
SHARE_DIR="/usr/share/kraftaEditor"
ASSETS_DIR="$SHARE_DIR/assets"
LANGUAGES_DIR="$SHARE_DIR/languages"
CONFIG_DIR="$SHARE_DIR/config"
I18N_DIR="$SHARE_DIR/i18n"
DESKTOP_FILE="/usr/share/applications/kraftaEditor.desktop"
ICON_PATH="$ASSETS_DIR/images/kraftaEditor.png"

echo "Installing $APP_NAME..."

if [[ $EUID -ne 0 ]]; then
    echo "Please run as root (use sudo)."
    exit 1
fi

PROJECT_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
BUILD_PATH="$PROJECT_ROOT/build/bin/RELEASE"
SOURCE_ASSETS="$PROJECT_ROOT/assets"
SOURCE_LANGUAGES="$PROJECT_ROOT/languages"
SOURCE_CONFIG="$PROJECT_ROOT/config"
SOURCE_I18N="$PROJECT_ROOT/i18n"

if [[ ! -f "$BUILD_PATH/$EXECUTABLE_NAME" ]]; then
    echo "Error: Executable not found at $BUILD_PATH/$EXECUTABLE_NAME"
    exit 1
fi

if [[ ! -d "$SOURCE_ASSETS" ]]; then
    echo "Error: Assets directory not found at $SOURCE_ASSETS"
    exit 1
fi

if [[ ! -d "$SOURCE_LANGUAGES" ]]; then
    echo "Error: Languages directory not found at $SOURCE_LANGUAGES"
    exit 1
fi

if [[ ! -d "$SOURCE_CONFIG" ]]; then
    echo "Error: Config directory not found at $SOURCE_CONFIG"
    exit 1
fi

if [[ ! -d "$SOURCE_I18N" ]]; then
    echo "Error: i18n directory not found at $SOURCE_I18N"
    exit 1
fi

echo "Copying executable to $INSTALL_DIR..."
install -Dm755 "$BUILD_PATH/$EXECUTABLE_NAME" "$INSTALL_DIR/$EXECUTABLE_NAME"

echo "Copying assets to $ASSETS_DIR..."
rm -rf "$ASSETS_DIR"
mkdir -p "$ASSETS_DIR"
cp -r "$SOURCE_ASSETS/"* "$ASSETS_DIR/"

echo "Copying languages to $LANGUAGES_DIR..."
rm -rf "$LANGUAGES_DIR"
mkdir -p "$LANGUAGES_DIR"
cp -r "$SOURCE_LANGUAGES/"* "$LANGUAGES_DIR/"

echo "Copying configs to $CONFIG_DIR..."
rm -rf "$CONFIG_DIR"
mkdir -p "$CONFIG_DIR"
cp -r "$SOURCE_CONFIG/"* "$CONFIG_DIR/"

echo "Copying i18n to $I18N_DIR..."
rm -rf "$I18N_DIR"
mkdir -p "$I18N_DIR"
cp -r "$SOURCE_I18N/"* "$I18N_DIR/"

echo "Creating desktop entry..."
cat <<EOF > "$DESKTOP_FILE"
[Desktop Entry]
Name=$APP_NAME
Exec=$EXECUTABLE_NAME
Icon=$ICON_PATH
Type=Application
Categories=Development;Utility;
Terminal=false
EOF

chmod 644 "$DESKTOP_FILE"
update-desktop-database >/dev/null 2>&1 || true

echo "$APP_NAME successfully installed!"
echo "You can launch it from your applications menu or by typing '$EXECUTABLE_NAME' in the terminal."
