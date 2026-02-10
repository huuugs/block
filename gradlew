#!/bin/bash
# Simple gradle wrapper that uses system gradle

if command -v gradle &> /dev/null; then
    # Forward all arguments to gradle
    gradle "$@"
else
    echo "Error: gradle not found in PATH"
    echo "Please install gradle or download gradle wrapper"
    exit 1
fi
