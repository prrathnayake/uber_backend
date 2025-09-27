# Docker Compose Validation Report

This report captures the expected outcomes when running the C++ microservices stack with `docker/docker-compose.yml`. It follows the rider and driver lifecycle described in `docs/docker_e2e_test_plan.md`, highlighting service readiness checks, critical API flows, and observable side effects across databases, Kafka, and RabbitMQ.

> **Note:** Docker is unavailable in this execution environment, so the observations below reflect deterministic behaviour derived from the source code and should be verified on a local machine with Docker Engine ≥ 24.

## 1. Topology & configuration

| Component | Container | Ports (host → container) | Notes |
| --- | --- | --- | --- |
| UserManager | `usermanager` | `8081 → 8081` | Builds from `docker/Dockerfile.UserManager` (Ubuntu + `docker/install_dependencies.sh`). Loads `.env` for DB + peer hostnames. 【F:docker/docker-compose.yml†L83-L118】【F:docker/Dockerfile.UserManager†L1-L12】【F:.env†L5-L17】 |
| RideManager | `ridemanager` | `8082 → 8082` | Depends on UserManager & LocationManager for rider/driver lookups and location matching. 【F:docker/docker-compose.yml†L120-L155】【F:RideManager/src/main.cpp†L17-L45】 |
| LocationManager | `locationmanager` | `8083 → 8083`, `50051 → 50051` | Runs HTTP + gRPC endpoints; consumes Kafka & RabbitMQ messages. 【F:docker/docker-compose.yml†L157-L185】【F:LocationManager/src/main.cpp†L1-L44】 |
| MySQL (per service) | `*-db` | `3307-3309 → 3306` | Health-checked instances per service. 【F:docker/docker-compose.yml†L23-L64】 |
| Kafka | `kafka` | `9092 → 9092`, `29092 → 29092` | Advertises `kafka:9092` internally, `localhost:29092` externally. 【F:docker/docker-compose.yml†L66-L80】 |
| RabbitMQ | `rabbitmq` | `5672 → 5672`, `15672 → 15672` | Used for driver notification fanout. 【F:docker/docker-compose.yml†L89-L101】 |
| Redis | `redis` | `6379 → 6379` | Optional cache; in-memory fallback is used if Redis is unreachable. 【F:docker/docker-compose.yml†L82-L87】【F:LocationManager/src/services/redisHandler/redisHandler.cpp†L6-L43】 |

### Startup checklist

Run the stack from the repository root:

```bash
docker compose -f docker/docker-compose.yml up --build
```

1. **Container health** – wait for `usermanager-db`, `ridemanager-db`, and `locationmanager-db` to report `healthy` (MySQL ping). 【F:docker/docker-compose.yml†L23-L64】
2. **Service logs** – confirm:
   - `usermanager`: `RouteHandler constructed.` when HTTP routes initialise. 【F:UserManager/src/services/routeHandler/routeHandler.cpp†L10-L34】
   - `ridemanager`: `RideRouteHandler::RideRouteHandler` debug log and Kafka/RabbitMQ hooks. 【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L39-L120】【F:RideManager/src/server.cpp†L50-L71】
   - `locationmanager`: `Processing location update payload` debug log once the first heartbeat is processed. 【F:LocationManager/src/services/routeHandler/routeHandler.cpp†L24-L64】
   - `rabbitmq`: queue declarations succeed (no errors).
3. **Environment overrides** – each binary loads `.env` so DB hostnames resolve to the MySQL containers while HTTP calls keep using the service names. 【F:sharedUtils/include/config.h†L60-L138】【F:UserManager/src/main.cpp†L17-L41】【F:RideManager/src/main.cpp†L17-L45】【F:LocationManager/src/main.cpp†L1-L44】

## 2. Onboarding & authentication flows

| Step | Request | Expected outcome |
| --- | --- | --- |
| Rider signup | `POST http://localhost:8081/signup` with complete profile payload | Returns `201` with `{"message":"Signup successful"}`. Validates mandatory fields, rejects duplicates, persists via `UserDBManager::addUserToDB`, and emits Kafka `user_created`. 【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L32-L120】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L12-L41】 |
| Driver signup | Same endpoint with `role=driver` | Creates driver record available to RideManager via `/user/{id}`. 【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L82-L120】 |
| Login | `POST /login` with username/password | Hashes password, compares to stored hash, returns JWT token (no expiry yet). Incorrect credentials yield `401`. 【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L42-L79】 |
| Update driver profile | `PUT /user/{id}` | Persists JSON diff with address/country updates. 【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L122-L171】 |
| Register vehicle metadata | `PATCH /user/{id}/profile` | Stores partial updates (e.g., make/model) in JSON columns. 【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L173-L205】 |

## 3. Location priming

1. **Driver heartbeat** – `POST http://localhost:8083/location/update` with user id + lat/lng. Response `200 {"status":"success"}`; invalid fields return `400` with descriptive error. 【F:LocationManager/src/services/httpHandler/servers/httpLocationServer.cpp†L20-L74】【F:LocationManager/src/services/routeHandler/routeHandler.cpp†L24-L120】
2. **Nearby search** – `GET /location/nearby?lat=...&lng=...&radius=3&role=driver` returns array of drivers sorted by distance (Haversine). 【F:LocationManager/src/services/httpHandler/servers/httpLocationServer.cpp†L42-L72】【F:LocationManager/src/services/routeHandler/routeHandler.cpp†L180-L236】
3. **Message queues** – LocationManager subscribes to Kafka `user_created` and RabbitMQ `location_updates`; check logs for consumed payloads. 【F:LocationManager/src/server.cpp†L21-L70】

## 4. Ride lifecycle

| Phase | Request | Expected behaviour |
| --- | --- | --- |
| Driver availability | `POST http://localhost:8082/drivers/{id}/status` with `{ "available": true }` | Updates in-memory availability, publishes Kafka `driver.status_changed`, returns driver snapshot (wallet balance, lifetime earnings). 【F:RideManager/src/services/httpHandler/servers/httpRideServer.cpp†L45-L68】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L620-L668】 |
| Request ride | `POST http://localhost:8082/rides/request` | Validates required fields; selects nearest available driver (LocationGateway fallback), transitions to `pending_driver`, emits Kafka (`ride.requested`, `ride.assigned`) and RabbitMQ driver notification, persists MySQL row. Returns `201` payload with generated `ride_id`. 【F:RideManager/src/services/httpHandler/servers/httpRideServer.cpp†L31-L44】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L440-L520】 |
| Accept ride | `POST /rides/{ride_id}/status` with `{ "status": "accepted" }` | Locks driver availability, updates DB row, emits `ride.status_changed` + RabbitMQ notification. Returns updated ride JSON. 【F:RideManager/src/services/httpHandler/servers/httpRideServer.cpp†L46-L60】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L520-L620】 |
| Complete ride | `POST /rides/{ride_id}/status` with `{ "status": "completed", "fare": 18.75, "payment_method": "wallet" }` | Credits driver wallet, records payment, unlocks driver, emits Kafka/RabbitMQ, returns final ride record with fare + payment status. Missing fare triggers warning and skips credit. 【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L520-L704】 |
| Post-ride checks | `GET /rides/{ride_id}` / `GET /drivers/{id}/profile` | Returns consolidated ride information and driver wallet snapshot (balance, lifetime earnings). 【F:RideManager/src/services/httpHandler/servers/httpRideServer.cpp†L68-L93】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L668-L704】 |

## 5. Cross-service observations

- **Kafka topics** – Inspect `docker compose logs kafka` or attach `kafka-console-consumer` to confirm `ride.*` and `driver.status_changed` events published by RideManager. 【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L492-L704】
- **RabbitMQ queue** – Use the management UI (`http://localhost:15672`) to monitor the `driver_notifications` queue populated during ride assignment/status changes. 【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L492-L704】
- **MySQL validation** – Inside the database containers, query `rides`, `users`, and `locations` tables to verify inserts/updates after each flow. 【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L452-L520】【F:LocationManager/src/services/routeHandler/routeHandler.cpp†L60-L160】

## 6. Cleanup

Stop the stack and remove volumes to reset databases:

```bash
docker compose -f docker/docker-compose.yml down -v
```

Re-running the suite from a clean slate ensures deterministic test runs for subsequent iterations.
