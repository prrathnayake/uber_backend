#!/usr/bin/env bash
set -euo pipefail

export DEBIAN_FRONTEND=noninteractive

apt-get update
apt-get install -y \
    build-essential \
    git \
    curl \
    wget \
    python3 \
    python3-pip \
    ninja-build \
    pkg-config \
    ca-certificates
rm -rf /var/lib/apt/lists/*

curl -LO https://github.com/Kitware/CMake/releases/download/v3.27.9/cmake-3.27.9-linux-x86_64.sh
chmod +x cmake-3.27.9-linux-x86_64.sh
./cmake-3.27.9-linux-x86_64.sh --skip-license --prefix=/usr/local
rm cmake-3.27.9-linux-x86_64.sh

pip3 install --no-cache-dir conan
conan profile detect --force

curl -Lo /usr/local/include/httplib.h https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h
