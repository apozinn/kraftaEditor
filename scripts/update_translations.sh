#!/bin/bash

DOMAIN="kraftaeditor"
POT_FILE="./locale/messages.pot"
PO_FILE="./locale/pt_BR.po"
LOCALE_DIR="./locale"

cd "$(dirname "$0")"

mkdir -p "$LOCALE_DIR"

find .. -type d \( -name "build" -o -name "bin" -o -name ".git" -o -name "third_party" \) -prune -o \
       -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) -print > files_to_scan.txt

xgettext --keyword=_ --language=C++ --from-code=UTF-8 \
         --sort-output \
         --package-name="$DOMAIN" \
         -f files_to_scan.txt -o "$POT_FILE"

if [ -f "$PO_FILE" ]; then
    msgmerge -U "$PO_FILE" "$POT_FILE"
else
    msginit --no-translator -l pt_BR -i "$POT_FILE" -o "$PO_FILE"
fi

rm files_to_scan.txt