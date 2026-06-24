#!/bin/bash

if pkg-config --exists raylib; then
    echo "Raylib is already installed."
    exit 0
fi

echo "Installing Raylib dependencies..."

sudo apt update

sudo apt install -y \
build-essential \
git \
cmake \
pkg-config \
libx11-dev \
libxrandr-dev \
libxi-dev \
libgl1-mesa-dev \
libglu1-mesa-dev \
libxcursor-dev \
libxinerama-dev

echo "Downloading Raylib..."

if [ ! -d "raylib" ]; then
    git clone https://github.com/raysan5/raylib.git
else
    echo "raylib folder already exists, using existing folder."
fi

cd raylib || exit 1

mkdir -p build
cd build || exit 1

echo "Building Raylib..."

cmake ..
make -j"$(nproc)"

echo "Installing Raylib..."

sudo make install
sudo ldconfig

echo "Raylib installed successfully."