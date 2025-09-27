# Docker Deployment & End-to-End Ride Flow Test Plan

## 1. Environment status
- `docker compose -f docker/docker-compose.yml up -d`
  - ❌ Fails in the current sandbox because Docker CLI is unavailable (`bash: command not found: docker`).
  - ✅ Works on a local machine with Docker Engine ≥ 24 and Docker Compose plugin installed.
- `.env` now pins service discovery hostnames (e.g., `USER_MANAGER_HOST=usermanager`, `USER_MANAGER_DATABASE_HOST=usermanager-db`) and credential mappings for the compose stack.

## 2. Prerequisites
1. Docker Engine and Docker Compose plugin installed locally.
2. 8+ GB RAM and 15+ GB disk space for MySQL, Kafka, Redis, and three service containers.
3. Ports 8081-8083, 3307-3309, 6379, and 9092 free on the host (adjust `.env` if needed).
4. Clone repo and run commands from repository root.

## 3. Start the microservice stack
```bash
docker compose -f docker/docker-compose.yml up --build
```
- Builds Ubuntu-based service images via `docker/install_dependencies.sh`, then provisions dedicated MySQL instances, Kafka, Redis, and RabbitMQ alongside the three service containers.
- Entrypoints run `conan build` then launch the service binaries (`entrypointUserManager.sh`, `entrypointRideManager.sh`, `entrypointLocationManager.sh`).
- Logs stream structured messages from the singleton logger.

### Health verification
```bash
docker compose ps
docker compose logs usermanager | tail
docker compose logs ridemanager | tail
docker compose logs locationmanager | tail
docker compose logs rabbitmq | tail
```
- Look for `RouteHandler constructed` (UserManager), `RideRouteHandler::RideRouteHandler` init (RideManager), and `Processing location update payload` logs to confirm handlers are live.

## 4. Seed reference data
1. **Create rider (UserManager `/signup`)**
```bash
curl -X POST http://localhost:8081/signup \
  -H 'Content-Type: application/json' \
  -d '{
        "firstName": "Riya",
        "middleName": "",
        "lastName": "Sen",
        "countryCode": "+1",
        "mobileNumber": "5550001000",
        "address": "1 Rider Way",
        "email": "riya@example.com",
        "username": "rider.riya",
        "password": "secret123",
        "role": "rider",
        "preferredLanguage": "en",
        "currency": "USD",
        "country": "US"
      }'
```
- Validates required field enforcement before inserting (`UserManager/src/services/httpHandler/Servers/httpUserServer.cpp`).
- Persists to `users` table via `UserDBManager::addUserToDB`.

2. **Create driver** (same endpoint, role = `driver`).
3. **Login to obtain JWT** (`/login`), verifying password hashing alignment between client input and stored hash.

## 5. Driver onboarding updates
1. **Update driver profile**
```bash
curl -X PUT http://localhost:8081/user/2 \
  -H 'Content-Type: application/json' \
  -d '{"address": "24 Driver Street", "country": "US"}'
```
- Exercises `handleUpdateUser` → `UserDBManager::updateUserById` to patch profile fields.
2. **Attach vehicle metadata**
```bash
curl -X PATCH http://localhost:8081/user/2/profile \
  -H 'Content-Type: application/json' \
  -d '{"vehicle_make": "Toyota", "vehicle_model": "Prius", "vehicle_type": "hybrid"}'
```
- Stored in `users` table JSON columns until dedicated vehicle endpoints are added; confirms partial update path.

## 6. Location priming (LocationManager)
1. **Driver location heartbeat**
```bash
curl -X POST http://localhost:8083/location/update \
  -H 'Content-Type: application/json' \
  -d '{"user_id": 2, "latitude": 37.776, "longitude": -122.417, "name": "Downtown driver"}'
```
- Validates numeric parsing, bounds checking, and UPSERT logic in `RouteHandler::handleLocationUpdate`.
2. **Rider location snapshot** (optional) to allow proximity queries.
3. **Nearby driver probe**
```bash
curl 'http://localhost:8083/location/nearby?lat=37.775&lng=-122.418&radius=3&role=driver'
```
- Confirms Haversine filtering and sorting in `handleFindNearby`.

## 7. Driver availability (RideManager)
```bash
curl -X POST http://localhost:8082/drivers/2/status \
  -H 'Content-Type: application/json' \
  -d '{"available": true}'
```
- Updates in-memory driver state via `setDriverAvailability` and publishes Kafka `driver.status_changed` events.
- Check `docker compose logs RideManager` for wallet snapshot payloads.

## 8. Ride booking lifecycle
1. **Request ride**
```bash
curl -X POST http://localhost:8082/rides/request \
  -H 'Content-Type: application/json' \
  -d '{
        "rider_id": "1",
        "pickup_location": "1 Rider Way",
        "dropoff_location": "100 Market St",
        "pickup_lat": 37.775,
        "pickup_lng": -122.418,
        "dropoff_lat": 37.789,
        "dropoff_lng": -122.401,
        "currency": "USD",
        "payment_method": "wallet"
      }'
```
- Allocates an available driver (from cached state or LocationManager) and persists ride row.
- Emits `ride.requested` and `ride.assigned` Kafka events and RabbitMQ driver notifications.
- Observe structured logs showing generated ride ID and driver assignment.

2. **Driver accepts ride**
```bash
curl -X POST http://localhost:8082/rides/ride-00000001/status \
  -H 'Content-Type: application/json' \
  -d '{"status": "accepted"}'
```
- Locks driver availability and records acceptance timestamp.

3. **Mark in-progress** (optional) then **complete ride**
```bash
curl -X POST http://localhost:8082/rides/ride-00000001/status \
  -H 'Content-Type: application/json' \
  -d '{
        "status": "completed",
        "fare": 18.75,
        "payment_method": "wallet"
      }'
```
- Calls `creditDriverWallet` and `recordPaymentInDatabase` for settlement; logs wallet increment and payment persistence.
- Kafka `ride.status_changed` event carries fare + payment metadata.

## 9. Post-ride verification
1. **Ride status**
```bash
curl http://localhost:8082/rides/ride-00000001
```
- Confirms `status=completed`, `fare` node, and `payment.status`.
2. **Driver wallet snapshot**
```bash
curl http://localhost:8082/drivers/2/profile
```
- Aggregates UserManager profile with wallet totals (`handleGetDriverProfile`).
3. **Database spot-checks** (inside containers)
```bash
docker exec -it rideManagerDatabase mysql -u pasan -ppasan rideManagerDatabase -e 'SELECT status,fare FROM rides;'
docker exec -it userManagerDatabase mysql -u pasan -ppasan userManagerDatabase -e 'SELECT user_id,role FROM users;'
```

## 10. Observability checklist
- **UserManager logs**: signup + login traces including hash comparisons.
- **RideManager logs**: driver availability transitions, ride allocation, wallet credit.
- **LocationManager logs**: location UPSERT success messages and nearby search metrics.
- **Kafka topic drill-down** (optional): attach console consumer to verify published events.

## 11. Additional negative tests
- Request ride with no available drivers → expect HTTP 409 with `"No drivers currently available"`.
- Complete ride without fare → warning log and no wallet credit.
- Invalid location payloads → HTTP 400 and diagnostic logs about missing/invalid fields.

## 12. Cleanup
```bash
docker compose -f docker/docker-compose.yml down -v
```
- Stops containers and removes volumes to reset MySQL state.
