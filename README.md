# 🚗 Uber Backend Clone (C++ Microservices) – Under Development

A **modular C++ backend system** inspired by Uber’s architecture. This project simulates a microservices environment to manage users, rides, and locations, using secure, scalable, and modern design patterns.

---

## 📦 Features

- 🧩 **Microservice Architecture**
  - Independent services: `UserManager`, `RideManager`, and `LocationManager`
- 💾 **Multi-DB Support**: MySQL, PostgreSQL, SQLite (per service)
- 🧵 **Thread Pool**: Asynchronous background task execution
- 🪵 **Color-Coded Logger**: Singleton-based centralized logging
- 🔐 **JWT Authentication**: Secure login/session using JSON Web Tokens
- 📬 **Message Brokers**:
  - Kafka: For inter-service events (`user_created`, etc.)
  - RabbitMQ: For async job/event delegation
- 📦 **Shared Libraries** via [`cpp_base`](https://github.com/prrathnayake/cpp-base)
- ⚙️ **Secure Configuration**: Via `.env` file + environment variables
- 🐳 **Dockerized**: Each microservice has its own Dockerfile & entrypoint
- 🔁 **CI/CD Ready**: GitHub Actions build and push Docker images

---

## 🏗️ Tech Stack

- **Language**: C++17 / C++20
- **Databases**: MySQL, PostgreSQL, SQLite
- **Message Brokers**: Apache Kafka, RabbitMQ
- **Networking**: gRPC + Protobuf
- **Build**: CMake + Conan
- **Containerization**: Docker + docker-compose
- **CI/CD**: GitHub Actions

---

## 📂 Project Structure

```plaintext
uber-backend/
├── UserManager/                     # Handles users + JWT auth
│   ├── include/                     # Headers
│   ├── src/                         # Source
│   └── sql_scripts/                # SQL init files
│
├── RideManager/                     # Handles ride matching and booking
├── LocationManager/                 # Geolocation services (H3-based)
│
├── sharedUtils/                     # Logger, thread pool, config loader
├── sharedResources/                 # Shared gRPC, Kafka, RabbitMQ, DB
├── proto/                           # gRPC proto definitions
├── docker/                          # Dockerfiles per service
│
├── entrypointUserManager.sh         # Entrypoint: UserManager
├── entrypointRideManager.sh         # Entrypoint: RideManager
├── entrypointLocationManager.sh     # Entrypoint: LocationManager
│
├── .env                             # Environment variables
├── docker-compose.yml               # Multi-service orchestration
├── CMakeLists.txt
├── conanfile.py
└── README.md
```

### 🚀 Getting Started

## 🛠️ Build & Run (Manual)

> 🛑 **Important:** Kafka and RabbitMQ must be running before you start the service binaries.

You can start Kafka and RabbitMQ using Docker (recommended) or your local installation.

### ✅ Start Kafka and RabbitMQ with Docker

```bash
docker-compose -f docker/docker-compose.message-stack.yml up -d
```

1. **Install dependencies** via Conan:

```bash
conan build . --output-folder=build --build=missing
```

2. **Run individual services:**
```bash
./build/Release/bin/UserManager
./build/Release/bin/RideManager
./build/Release/bin/LocationManager
```

## 🚀 Getting Started

```bash
docker-compose up --build
```

