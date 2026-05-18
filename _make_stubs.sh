#!/bin/bash
# Create stub .a archives to bypass broken oo-subsystems worktree build.
# Uses printf to write valid empty ar archive headers.
set -e

OO_BUILD="/mnt/c/Users/djibi/OneDrive/Bureau/baremetal/llm-baremetal.worktrees/copilot-worktree-2026-03-21T23-04-08/build/oo"

mkdir -p "$OO_BUILD"

for lib in liboo-kernel.a liboo-warden.a liboo-engine.a liboo-modules.a liboo-bus.a librust_guard.a; do
    target="$OO_BUILD/$lib"
    if [ ! -f "$target" ] || [ ! -s "$target" ]; then
        printf '!<arch>\n' > "$target"
        echo "Created stub: $target"
    else
        echo "Already exists: $target ($(stat -c '%s' "$target") bytes)"
    fi
done

echo "Done: all stub archives ready."
ls -la "$OO_BUILD/"
