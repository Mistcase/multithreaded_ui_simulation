#!/usr/bin/env bash
set -euo pipefail

# Directory to clean (default: current directory)
ROOT_DIR="${1:-.}"

# Remove regular backup files: file~
find "$ROOT_DIR" -type f -name '*~' -print -delete

# Remove autosave files: #file#
find "$ROOT_DIR" -type f -name '#*#' -print -delete

# Remove Emacs lock files: .#file
find "$ROOT_DIR" -type f -name '.#*' -print -delete
