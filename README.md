# 🚗 Uber Backend Clone

A **modular backend system** built in modern **C++**, inspired by Uber's architecture. This clone showcases core backend features, including database integration, multithreaded task execution, and centralized logging, designed for educational, portfolio, or experimental use.

---

## 📦 Features

- 🔧 **Modular architecture** with clearly separated concerns (e.g., server, database, logging).
- 💾 **MySQL** integration using a custom database wrapper.
- 🧵 **Thread pool** for task queuing and background execution.
- 🪵 **Centralized singleton logger** for uniform log handling across all modules.
- 📂 Environment variables and secrets management for secure configs.
- 🧱 Built using a **custom utility library: [`cpp_base`](https://github.com/prrathnayake/cpp-base)**, offering:
  - Singleton logger
  - Thread pool
  - Secrets/config loader
  - General utility modules

---

## 🏗️ Built With

- **C++17/20**
- [cpp_base](https://github.com/prrathnayake/cpp-base) (custom utility library)
- **MySQL Connector/C++**
- **CMake**
- **Conan** (for dependency and build management)

---

## 📁 Project Structure
```plaintext
uber-backend/
├── include/                 # Project headers
│   ├── server.h
│   └── database/
│       └── database.h
├── src/                     # Source files
│   ├── main.cpp
│   ├── server.cpp
│   └── database/
│       └── database.cpp
├── utils/                   # From cpp_base (or integrated)
│   ├── log/
│   │   └── singletonLogger.{h,cpp}
│   └── threadpool/
│       └── threadpool.{h,cpp}
├── sql_scripts/             # SQL for database initialization
├── log/                     # Log output directory
├── CMakeLists.txt
└── README.md
```

## 🚀 Getting Started

1. **Install dependencies** via Conan:

```bash
conan install . --output-folder=build --build=missing
cd build
cmake ..
cmake --build .
./bin/uber-backend
```


