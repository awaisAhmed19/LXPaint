#!/bin/bash

set -e

# Create build dir if it doesn't exist
if [ ! -d "build" ]; then
    echo "📁 Creating build directory..."
    mkdir build
    cd build
    cmake ..
else
    cd build
fi

echo "🔨 Building..."
make -j$(nproc)

echo " Running..."
./lxpaint
