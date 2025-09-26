# 🚗 Uber Backend Clone (C++ Microservices) – Under Development

A **modular C++ backend system** inspired by Uber’s architecture. This project simulates a microservices environment to manage users, rides, and locations, using secure, scalable, and modern design patterns.

---

![Untitled Diagram(10)](https://github.com/user-attachments/assets/02bb3d93-590f-4973-9c9c-e2b3b13d0ac0)

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
  - RabbitMQ: For async job/event delegation with an in-memory fallback to simplify local testing
- 📦 **Shared Libraries** via [`cpp_base`](https://github.com/prrathnayake/cpp-base)
- ⚙️ **Secure Configuration**: Via `.env` file + environment variables
- 🐳 **Dockerized**: Each microservice has its own Dockerfile & entrypoint
- 🔁 **CI/CD Ready**: GitHub Actions build and push Docker images
- 🌐 **gRPC Services**: LocationManager boots its own gRPC endpoint alongside HTTP handlers

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

# 🧠 Microservice Architecture – Uber Backend Clone (C++)

---

## 🖥️ Server Lifecycle

Each microservice follows this initialization flow:

1. **Database Initialization**  
   Initializes a dedicated SQL database for the service. This stores all persistent data relevant to the service domain.

2. **HTTP Handler Launch**  
   Sets up HTTP servers and clients, routes incoming requests to proper controllers, and manages outgoing requests.

3. **Kafka Handler Activation**  
   Initializes Kafka producers and consumers for event-driven communication between microservices.

---

## 🔍 Component Descriptions

- **🗄️ Database**  
  Dedicated storage per service using MySQL, PostgreSQL, or SQLite.

- **🌐 HTTP Handler**  
  Manages HTTP servers and clients. Routes requests to appropriate handlers.

- **📥 HTTP Server**  
  Listens for external or internal HTTP requests and serves responses based on API logic.

- **📤 HTTP Client**  
  Sends HTTP requests to other microservices to fetch or send data.

- **🧭 Kafka Handler**  
  Coordinates Kafka producer and consumer setup within the service.

- **📩 Kafka Consumer**  
  Subscribes to topics and listens for inter-service messages. Integrates incoming data into the service's workflow.

- **📨 Kafka Producer**  
  Publishes events/messages to Kafka topics for other microservices to consume.

---

## 🚗 Microservices Breakdown

---

### 1. UserManager Server
**Purpose:** Manage all user-related operations

**Responsibilities:**
- Handle user registration and login
- Manage user profiles (name, email, phone)
- Generate JWT tokens and hash passwords (bcrypt/Argon2)
- Publish events to Kafka/RabbitMQ (e.g., user signup)
- Expose HTTP endpoints:
  - `/register`
  - `/login`
  - `/profile`

---

### 2. RideManager Server
**Purpose:** Handle ride requests, assignments, and tracking

**Responsibilities:**
- Accept and manage ride requests
- Assign drivers based on proximity and availability
- Track ride status (`requested → accepted → in-progress → completed`)
- Publish/consume Kafka events (e.g., ride started, completed)
- Expose HTTP APIs:
  - `/requestRide`
  - `/rideStatus`
  - `/cancelRide`

---

### 3. LocationManager Server
**Purpose:** Handle real-time geolocation tracking

**Responsibilities:**
- Track and update driver/rider locations
- Use Uber H3 for geospatial indexing
- Find nearby drivers or riders
- Publish/consume location updates via Kafka/RabbitMQ
- Provide real-time location services

---

## 🔧 Common Subsystems Used in All Servers

- 🗃️ **Dedicated SQL Database** (MySQL/PostgreSQL/SQLite)
- 🌐 **HTTP Server** for exposing REST APIs
- 📡 **Kafka Handler** for event messaging (Producer + Consumer)
- 📬 **RabbitMQ Handler** (optional command queue with thread-safe in-memory fallback)
- 🌐 **gRPC Bootstrap Helpers** (background server lifecycle management)
- 🔁 **gRPC Client/Server** (optional internal communication)
- 🧵 **Thread Pool** for async task execution
- 📋 **Singleton Logger** with colored output
- 🔐 **.env / Env Variable Config Loader**

---

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

## 🐳 Docker Compose Setup

This project supports a fully containerized microservices environment using **Docker Compose**.

---

### 📦 Services

| Service                  | Description                               | Port        |
|--------------------------|-------------------------------------------|-------------|
| `mysql_userManagerDatabase` | MySQL for UserManager                    | `${USERMANAGER_PORT}` |
| `mysql_rideManagerDatabase` | MySQL for RideManager                    | `${RIDEMANAGER_PORT}` |
| `mysql_locationManagerDatabase` | MySQL for LocationManager            | `${LOCATIONMANAGER_PORT}` |
| `kafka`                  | Kafka broker using Bitnami (KRaft mode)   | `9092`      |
| `redis`                  | Redis instance (for caching/session)      | `6379`      |
| `UserManager`            | UserManager C++ microservice              | `${USERMANAGER_APP_PORT}` |
| `RideManager`            | RideManager C++ microservice              | `${RIDEMANAGER_APP_PORT}` |
| `LocationManager`        | LocationManager C++ microservice          | `${LOCATIONMANAGER_APP_PORT}` |

---

### 🔧 Environment Configuration

Set your values in a `.env` file at the project root:

```env
# MySQL shared config
MYSQL_ROOT_PASSWORD=yourRootPassword
MYSQL_USER=uber
MYSQL_PASSWORD=securepass

# Databases
USERMANAGER_DB=userdb
RIDEMANAGER_DB=ridedb
LOCATIONMANAGER_DB=locationdb

# DB Ports
USERMANAGER_PORT=3307
RIDEMANAGER_PORT=3308
LOCATIONMANAGER_PORT=3309

# Application Ports
USERMANAGER_APP_PORT=8081
RIDEMANAGER_APP_PORT=8082
LOCATIONMANAGER_APP_PORT=8083

# gRPC Ports
LOCATION_MANAGER_GRPC_PORT=50051

# RabbitMQ
RABBITMQ_HOST=localhost
RABBITMQ_PORT=5672
RABBITMQ_USERNAME=guest
RABBITMQ_PASSWORD=guest
RABBITMQ_VHOST=/
```

### 🔧 Run Docker Compose

```bash
docker-compose up --build
```


---

## 🤝 Community & Project Standards

- Review the [Code of Conduct](CODE_OF_CONDUCT.md) to understand the expectations for participating in this community.
- See the [Contributing Guide](CONTRIBUTING.md) for instructions on setting up your environment, coding standards, and submitting pull requests.
- Consult the [Security Policy](SECURITY.md) for guidance on how to responsibly disclose vulnerabilities.
- This project is distributed under the terms of the [MIT License](LICENSE).
