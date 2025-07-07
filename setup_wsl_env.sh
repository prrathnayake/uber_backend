#!/bin/bash
set -e

# === Update and install system packages ===
echo "=== Updating packages ==="
sudo apt-get update && sudo apt-get install -y \
    build-essential \
    git \
    curl \
    wget \
    cmake \
    pkg-config \
    lsb-release \
    software-properties-common \
    python3 \
    python3-pip \
    unzip \
    uuid-dev \
    libsasl2-dev \
    libssl-dev \
    libpq-dev \
    libmysqlclient-dev \
    libsqlite3-dev \
    librabbitmq-dev \
    librdkafka-dev \
    libcurl4-openssl-dev \
    autoconf \
    libtool \
    protobuf-compiler \
    libc-ares-dev \
    pigz \
    zlib1g \
    ca-certificates \
    gnupg \
    apt-transport-https \
    jq \
    dos2unix \
    mysql-server \
    && sudo rm -rf /var/lib/apt/lists/*

# === Install latest CMake ===
echo "=== Installing latest CMake ==="
sudo apt-get purge --auto-remove -y cmake
curl -LO https://github.com/Kitware/CMake/releases/download/v3.27.9/cmake-3.27.9-linux-x86_64.sh
chmod +x cmake-3.27.9-linux-x86_64.sh
sudo ./cmake-3.27.9-linux-x86_64.sh --skip-license --prefix=/usr/local
rm cmake-3.27.9-linux-x86_64.sh

# === Install Docker ===
echo "=== Installing Docker (WSL compatible) ==="
sudo mkdir -p /etc/apt/keyrings
curl -fsSL https://download.docker.com/linux/ubuntu/gpg | sudo gpg --dearmor -o /etc/apt/keyrings/docker.gpg
echo \
  "deb [arch=$(dpkg --print-architecture) signed-by=/etc/apt/keyrings/docker.gpg] https://download.docker.com/linux/ubuntu \
  $(lsb_release -cs) stable" | sudo tee /etc/apt/sources.list.d/docker.list > /dev/null
sudo apt-get update
sudo apt-get install -y docker-ce docker-ce-cli containerd.io docker-buildx-plugin docker-compose-plugin
sudo usermod -aG docker "$USER"

# === Enable and start MySQL ===
echo "=== Initializing MySQL Server ==="
sudo service mysql start
sudo mysql -e "ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY 'root'; FLUSH PRIVILEGES;"

# === Install Uber H3 ===
echo "=== Installing Uber H3 ==="
git clone https://github.com/uber/h3.git /tmp/h3
cd /tmp/h3 && mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local -DCMAKE_POSITION_INDEPENDENT_CODE=ON
make -j$(nproc) && sudo make install
cd / && rm -rf /tmp/h3

# === Build librdkafka ===
echo "=== Building librdkafka ==="
git clone https://github.com/confluentinc/librdkafka.git /tmp/librdkafka
cd /tmp/librdkafka && mkdir build && cd build
cmake ..
make -j$(nproc) && sudo make install
cd / && rm -rf /tmp/librdkafka

# === Build rabbitmq-c ===
echo "=== Building rabbitmq-c ==="
git clone https://github.com/alanxz/rabbitmq-c.git /tmp/rabbitmq-c
cd /tmp/rabbitmq-c && mkdir build && cd build
cmake .. -DBUILD_STATIC_LIBS=ON -DBUILD_SHARED_LIBS=ON -DENABLE_SSL_SUPPORT=ON -DBUILD_TESTS=OFF
make -j$(nproc) && sudo make install
cd / && rm -rf /tmp/rabbitmq-c

# === Install nlohmann_json ===
echo "=== Installing nlohmann_json ==="
git clone https://github.com/nlohmann/json.git /tmp/json
cd /tmp/json && mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local
sudo make install
cd / && rm -rf /tmp/json

# === Install cpp-httplib ===
echo "=== Installing cpp-httplib ==="
sudo curl -Lo /usr/local/include/httplib.h https://raw.githubusercontent.com/yhirose/cpp-httplib/master/httplib.h

# === Install AMQP-CPP ===
echo "=== Installing AMQP-CPP ==="
git clone https://github.com/akalend/amqpcpp.git /tmp/amqpcpp
git clone https://github.com/alanxz/rabbitmq-c.git /tmp/amqpcpp/rabbitmq-c
cd /tmp/amqpcpp && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DAMQP-CPP_BUILD_SHARED=ON
make -j$(nproc) && sudo make install
sudo cp libamqpcpp.so /usr/local/lib
sudo cp -r ../include/* /usr/local/include
cd /
git clone https://github.com/CopernicaMarketingSoftware/AMQP-CPP.git /tmp/AMQP-CPP
cd /tmp/AMQP-CPP && mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DAMQP-CPP_BUILD_SHARED=ON
make -j$(nproc) && sudo make install
cd / && sudo cp /tmp/amqpcpp/build/libamqpcpp.so /usr/local/lib

# === Fix AMQP include path ===
echo "=== Fixing AMQP include path ==="
sudo mkdir -p /usr/local/include/rabbitmq-c
sudo ln -sf /usr/local/include/AMQPcpp.h /usr/local/include/rabbitmq-c/AMQPcpp.h

# === Build gRPC ===
echo "=== Building gRPC ==="
cd /tmp
git clone --recurse-submodules -b v1.63.0 https://github.com/grpc/grpc
cd grpc && mkdir -p cmake/build && cd cmake/build
cmake ../.. -DCMAKE_BUILD_TYPE=Release \
    -DgRPC_INSTALL=ON \
    -DgRPC_BUILD_TESTS=OFF \
    -DgRPC_PROTOBUF_PROVIDER=module \
    -DgRPC_ZLIB_PROVIDER=package \
    -DgRPC_CARES_PROVIDER=module \
    -DgRPC_ABSL_PROVIDER=module \
    -DgRPC_RE2_PROVIDER=module
make -j$(nproc) && sudo make install
cd / && rm -rf /tmp/grpc

# === Install zlib 1.3.1 ===
echo "=== Installing zlib 1.3.1 ==="
cd /tmp
wget http://zlib.net/zlib-1.3.1.tar.gz
tar -xvzf zlib-1.3.1.tar.gz && cd zlib-1.3.1
./configure && make && sudo make install
echo "/usr/local/lib" | sudo tee /etc/ld.so.conf.d/zlib.conf
sudo ldconfig
cd /tmp && rm -rf zlib-1.3.1*

# === Rebuild pigz ===
echo "=== Rebuilding pigz ==="
git clone https://github.com/madler/pigz.git /tmp/pigz
cd /tmp/pigz && make
sudo cp pigz unpigz /usr/local/bin
rm -rf /tmp/pigz

# === Configure Environment Variables ===
echo "=== Configuring Environment Variables ==="
echo 'export CMAKE_PREFIX_PATH=/usr/local:/usr/local/lib/cmake:/usr/local/lib64/cmake:/usr/local/lib/cmake/RdKafka' >> ~/.bashrc
echo 'export CMAKE_MODULE_PATH=/usr/local/lib/cmake' >> ~/.bashrc
echo 'export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:/usr/lib/pkgconfig:/usr/lib/x86_64-linux-gnu/pkgconfig' >> ~/.bashrc
echo 'export LD_LIBRARY_PATH=/usr/local/lib:/usr/lib:/usr/lib/x86_64-linux-gnu' >> ~/.bashrc
echo 'export CPLUS_INCLUDE_PATH=/usr/local/include:/usr/local/include/rabbitmq-c' >> ~/.bashrc
source ~/.bashrc
sudo ldconfig

# === Install Conan ===
echo "=== Installing Conan ==="
pip3 install --break-system-packages conan
conan profile detect --force

# === Clone and Build Project Modules ===
echo "=== Cloning C++ modules ==="
git clone https://github.com/prrathnayake/cpp-tools.git /opt/cpp-tools
git clone https://github.com/prrathnayake/cpp-base.git /opt/cpp-base

echo "=== Building and Exporting cpp-tools ==="
conan build /opt/cpp-tools --build=missing
conan export /opt/cpp-tools --user=pasan --channel=testing

echo "=== Building cpp-base ==="
python3 /opt/cpp-tools/module/scripts/buildBase.py /opt/cpp-base

# === Final Message ===
echo "\n✅ Full WSL Ubuntu C++ Dev Environment Setup Complete. Please restart your terminal or run 'newgrp docker' to apply Docker group permissions."
