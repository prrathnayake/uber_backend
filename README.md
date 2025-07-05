# 🚗 Uber Backend Clone (C++ Microservices)

A **modular C++ backend system** inspired by Uber’s architecture. This clone simulates microservices for handling user, ride, and location data. It integrates logging, database management, multithreading, and secure service configuration.

---

## 📦 Features

- 🧩 **Microservice architecture**:
  - `UserManager`, `RideManager`, and `LocationManager` are independent services.
- 💾 **Database support** for MySQL, PostgreSQL, and SQLite (per-service).
- 🧵 **Thread pool** for background execution of tasks.
- 🪵 **Singleton-based logger** for consistent, colored logging.
- 🧱 **Shared utilities** via [`cpp_base`](https://github.com/prrathnayake/cpp-base).
- 🛡️ **Secure configuration** via environment variables and `.env` file.
- 🐳 **Docker-ready** with separate entrypoints for each microservice.

---

## 🏗️ Tech Stack

- **C++17 / C++20**
- **MySQL Connector/C++**, PostgreSQL, SQLite
- **gRPC** + Protobuf
- **CMake**, **Conan**
- **Docker**, **GitHub Actions**

---

## 🗂️ Project Structure

```plaintext
uber-backend/
├── UserManager/                     # User service
│   ├── include/
│   │   ├── database/
│   │   ├── models/
│   │   ├── services/
│   │   └── server.h
│   ├── src/
│   │   ├── database/
│   │   ├── models/
│   │   ├── services/
│   │   ├── main.cpp
│   │   └── server.cpp
│   └── sql_scripts/
│
├── RideManager/                     # Ride service
├── LocationManager/                 # Location service
│
├── sharedUtils/                     # Logger, thread pool, config loader
├── sharedResources/                 # Shared components (e.g., gRPC setup)
├── proto/                           # gRPC protobuf definitions
├── docker/                          # Dockerfiles for each service
│
├── entrypointUserManager.sh         # Entrypoint for UserManager
├── entrypointRideManager.sh         # Entrypoint for RideManager
├── entrypointLocationManager.sh     # Entrypoint for LocationManager
│
├── .env                             # Environment config
├── CMakeLists.txt
├── conanfile.py
└── README.md
```

## 🚀 Getting Started

1. **Install dependencies** via Conan:

```bash
conan build . --output-folder=build --build=missing
```

## 🚀 Getting Started

```bash
docker-compose up --build
```

