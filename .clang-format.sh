#!/bin/bash

STYLE=$(git config --get hooks.clangformat.style)
STYLE_ARG="-style=file"
if [ -n "$STYLE" ]; then
    STYLE_ARG="-style=$STYLE"
fi

FormatFile() {
    clang-format -i $STYLE_ARG "$1"
    git add "$1"
}


case $1 in
    --about)
        echo Utility to run clang-format on source files
        ;;
    --all)
        for file in $(find Src -type f | grep -E \\.\(h\|cpp\)\$); do
            FormatFile "$file"
        done
        ;;
    *)
        for file in $(git diff-index --cached --name-only HEAD Src/*.h Src/*.cpp); do
            FormatFile "$file"
        done
        ;;
esac
