#!/bin/bash
#
# Fetches cocos2d-x 2.2.6 into <repo>/cocos2d-x, patches it for the current
# macOS SDK, and checks out the lib-dxco submodule. Safe to re-run: it skips the
# download if the tree is already there and skips patches already applied.

set -euo pipefail

PROJ_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(dirname "$PROJ_DIR")"
COCOS_DIR="$REPO_DIR/cocos2d-x"
PATCH_FILE="$PROJ_DIR/patches/cocos2d-x-2.2.6-macos.patch"
TARBALL_URL="https://codeload.github.com/cocos2d/cocos2d-x/tar.gz/refs/tags/cocos2d-x-2.2.6"

# Applies $2 inside $1, tolerating an already-patched tree.
apply_patch() {
    local dir="$1" file="$2"
    # -N -t keeps patch non-interactive: it never asks whether to reverse a hunk.
    if patch -p1 -d "$dir" -N -t -s --dry-run < "$file" >/dev/null 2>&1; then
        patch -p1 -d "$dir" -N -t -s < "$file"
        echo "    applied $(basename "$file")"
    elif patch -p1 -d "$dir" -R -t -s --dry-run < "$file" >/dev/null 2>&1; then
        echo "    $(basename "$file") already applied, skipping"
    else
        echo "    ERROR: $(basename "$file") neither applies nor appears applied in $dir" >&2
        return 1
    fi
}

if [ ! -f "$COCOS_DIR/cocos2dx/cocos2d.cpp" ]; then
    echo "==> Downloading cocos2d-x 2.2.6 (~55 MB)"
    TMP_DIR="$(mktemp -d)"
    trap 'rm -rf "$TMP_DIR"' EXIT

    curl -fL -o "$TMP_DIR/cocos2d-x.tar.gz" "$TARBALL_URL"
    tar xzf "$TMP_DIR/cocos2d-x.tar.gz" -C "$TMP_DIR"
    mv "$TMP_DIR/cocos2d-x-cocos2d-x-2.2.6" "$COCOS_DIR"
else
    echo "==> cocos2d-x already present at $COCOS_DIR"
fi

echo "==> Patching cocos2d-x for the current macOS SDK"
apply_patch "$COCOS_DIR" "$PATCH_FILE"

echo "==> Fetching the lib-dxco submodule"
if [ ! -f "$REPO_DIR/Classes/dxco/Action.cpp" ]; then
    git -C "$REPO_DIR" submodule update --init
else
    echo "    already checked out"
fi

echo
echo "Done. Build and run with:"
echo "  cmake -S proj.mac -B proj.mac/build -DCMAKE_BUILD_TYPE=Release"
echo "  cmake --build proj.mac/build -j8"
echo "  open 'proj.mac/build/Panda vs Zombies.app'"
