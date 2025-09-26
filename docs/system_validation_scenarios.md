# Uber Backend Scenario Validation Playbook

This playbook enumerates end-to-end scenarios that exercise every HTTP, messaging, and integration path exposed by the C++ microservices in this repository. Each scenario lists the exact request to issue, the expected response contract, and any observable side effects (database writes, Kafka topics, RabbitMQ queues, or gRPC calls) so that you can validate requirements step-by-step.

## Prerequisites

1. Start the shared infrastructure (Kafka, RabbitMQ, MySQL instances) and every service binary or Docker compose stack as described in the main README.
2. Ensure configuration values (ports, hosts, secrets) in `.env` match the running environment.
3. Reset databases if you want deterministic identifiers when replaying scenarios.
4. The examples below assume JSON over HTTP. Replace placeholder identifiers (e.g., `{{USER_ID}}`) with actual IDs returned earlier in the flow.

---

## Scenario 1 – Rider Account Onboarding & Authentication

Purpose: Validate all `UserManager` flows that a new rider exercises from discovery through account maintenance.

| Step | Request | Expected outcome |
| --- | --- | --- |
| 1 | `GET /user/exists?username=newrider&email=rider@example.com` | `200 OK` with JSON flags `{"usernameTaken": false, "emailTaken": false}` confirming availability before signup.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L137-L169】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L102-L129】 |
| 2 | `POST /signup` with body containing required profile fields and plaintext password | `201 Created` with message `Signup successful`. Database row is inserted and a Kafka `user_created` event is produced.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L171-L236】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L37-L67】【F:UserManager/src/services/kafkaHandler/producers/userKafkaManager.cpp†L18-L48】 |
| 3 | `POST /login` with username/password | `200 OK` returning JWT `token` and `user_id`. Password is hashed and verified against stored hash.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L83-L135】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L69-L85】 |
| 4 | `GET /user/me` with `Authorization: Bearer <token>` | `200 OK` returning the persisted user JSON. Token subject extracted from JWT and looked up by ID.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L311-L336】 |
| 5 | `GET /user/{{USER_ID}}` | `200 OK` echoing the same profile payload as step 4; `404` if missing.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L238-L253】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L87-L94】 |
| 6 | `PUT /user/{{USER_ID}}` with updated profile JSON | `200 OK` message `User updated successfully`, DB row updated, and Kafka `user_updated` event emitted.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L255-L276】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L96-L118】【F:UserManager/src/services/kafkaHandler/producers/userKafkaManager.cpp†L50-L64】 |
| 7 | `PATCH /user/{{USER_ID}}/profile` with partial JSON (e.g., `{ "preferredLanguage": "en" }`) | `200 OK` message `Profile updated` after targeted column update. Expect Kafka `user_updated` emission when RouteHandler extends to call the producer for partial updates (no-op today if not wired).【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L303-L310】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L140-L148】 |
| 8 | `PATCH /user/{{USER_ID}}/password` with `oldPassword`/`newPassword` | `200 OK` message `Password updated` when hashes match. Otherwise `400` and no change.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L276-L302】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L120-L139】 |
| 9 | `POST /user/token/introspect` with `{ "token": "<token>" }` | `200 OK` JSON `{ "valid": true, "user_id": "{{USER_ID}}" }` verifying JWT integrity; `401` when invalid.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L348-L370】 |
| 10 | `POST /user/refresh-token` with `{ "token": "<token>" }` | `200 OK` with a refreshed token and subject; `401` when expired/invalid.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L324-L347】 |
| 11 | `POST /user/logout` | `200 OK` message `Logged out` (stateless acknowledgement).【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L374-L378】 |
| 12 | `GET /health` | `200 OK` diagnostics summarising DB probe latency, Kafka configuration, and environment metadata.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L372-L373】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L150-L198】 |

---

## Scenario 2 – Operations Staff User Administration

Purpose: Exercise list, pagination, search, statistics, and deletion workflows used by internal operators.

| Step | Request | Expected outcome |
| --- | --- | --- |
| 1 | `GET /users` | `200 OK` with array of all users.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L288-L294】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L110-L117】 |
| 2 | `GET /users/paginated?offset=0&limit=10` | `200 OK` JSON `{ "data": [...], "meta": {"offset":0,"limit":10} }` with server-side bounds on `limit` (1–100).【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L296-L307】 |
| 3 | `GET /users/stats` | `200 OK` aggregated counters (totals by role, active accounts, etc.) as emitted by `getUserStats()`.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L309-L313】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L130-L134】 |
| 4 | `GET /user/search?username=rider` | `200 OK` array of fuzzy matches on username.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L312-L323】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L136-L143】 |
| 5 | `DELETE /user/{{USER_ID}}` | `200 OK` message `User deleted successfully` and Kafka `user_deleted` event; `404` when record absent.【F:UserManager/src/services/httpHandler/Servers/httpUserServer.cpp†L278-L287】【F:UserManager/src/services/routeHandler/routeHandler.cpp†L118-L119】【F:UserManager/src/services/kafkaHandler/producers/userKafkaManager.cpp†L66-L74】 |

---

## Scenario 3 – Driver Availability & Location Priming

Purpose: Prime the fleet state before rides are requested.

| Step | Request | Expected outcome |
| --- | --- | --- |
| 1 | `POST /location/update` with `{ "user_id": 501, "latitude": 37.7749, "longitude": -122.4194, "name": "Driver 501" }` | `201 Created` when inserting a new row; `200 OK` with message `Location updated` when overwriting. Payload validated for numeric IDs and coordinate ranges before persistence.【F:LocationManager/src/services/httpHandler/servers/httpLocationServer.cpp†L20-L47】【F:LocationManager/src/services/routeHandler/routeHandler.cpp†L24-L153】 |
| 2 | `POST /location/update` for multiple drivers across map | Each call upserts coordinates for the driver/rider pair. Use to seed availability pool.【F:LocationManager/src/services/routeHandler/routeHandler.cpp†L104-L150】 |
| 3 | `POST /drivers/{{DRIVER_ID}}/status` with `{ "available": true }` | `200 OK` response `{"status":"success","message":"Driver status updated","data":{"driver_id":"{{DRIVER_ID}}","available":true}}`. Driver availability tracked in-memory and broadcast to Kafka `driver.status_changed`.【F:RideManager/src/services/httpHandler/servers/httpRideServer.cpp†L56-L78】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L298-L329】【F:RideManager/src/services/kafkaHandler/rideKafkaManager.cpp†L32-L47】 |
| 4 | Optional: consume RabbitMQ queue `driver_notifications` to verify no notifications until a ride is assigned.【F:RideManager/src/services/rabbitHandler/rideRabbitManager.cpp†L31-L47】 |

---

## Scenario 4 – Happy Path Ride Fulfilment

Purpose: Validate the complete ride lifecycle from request through completion.

| Step | Request | Expected outcome |
| --- | --- | --- |
| 1 | `POST /rides/request` with rider ID, pickup/dropoff addresses, and coordinates | `201 Created` with generated `ride_id`, assigned `driver_id`, status `pending_driver`, and persisted record. Side effects: driver marked busy, ride saved to DB, Kafka emits `ride.requested` and `ride.assigned`, RabbitMQ notifies driver, gRPC client logs `SendLocation`. If no drivers available the API returns `409` with `No drivers currently available`.【F:RideManager/src/services/httpHandler/servers/httpRideServer.cpp†L34-L53】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L53-L200】【F:RideManager/src/services/kafkaHandler/rideKafkaManager.cpp†L32-L47】【F:RideManager/src/services/rabbitHandler/rideRabbitManager.cpp†L31-L47】【F:sharedResources/src/sharedgRPCClienth.cpp†L6-L26】 |
| 2 | `POST /rides/{{RIDE_ID}}/status` with `{ "status": "accepted" }` | `200 OK` success payload reflecting updated status, optional `reason`. Driver remains unavailable. Kafka `ride.status_changed` event and RabbitMQ notification fired.【F:RideManager/src/services/httpHandler/servers/httpRideServer.cpp†L55-L72】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L202-L276】 |
| 3 | `POST /rides/{{RIDE_ID}}/status` with `{ "status": "in_progress" }` | `200 OK` showing state transition; no driver availability change.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L222-L248】 |
| 4 | `POST /rides/{{RIDE_ID}}/status` with `{ "status": "completed", "fare": "$23.50" }` | `200 OK` final record, driver availability reset to `true`, Kafka and RabbitMQ emissions continue.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L248-L276】 |
| 5 | `GET /rides/{{RIDE_ID}}` | `200 OK` with full ride JSON echoing stored fields. `404` if ride ID invalid.【F:RideManager/src/services/httpHandler/servers/httpRideServer.cpp†L74-L83】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L331-L342】 |
| 6 | `GET /rides/user/{{RIDER_ID}}` | `200 OK` array of rides for rider sorted by insertion order.【F:RideManager/src/services/httpHandler/servers/httpRideServer.cpp†L85-L94】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L344-L360】 |
| 7 | `GET /rides/driver/{{DRIVER_ID}}` | `200 OK` array of rides handled by driver.【F:RideManager/src/services/httpHandler/servers/httpRideServer.cpp†L96-L105】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L362-L374】 |
| 8 | `GET /drivers/{{DRIVER_ID}}/profile` | `200 OK` with driver profile pulled live from `UserManager`; `404` when user missing.【F:RideManager/src/services/httpHandler/servers/httpRideServer.cpp†L107-L116】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L172-L199】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L376-L386】 |

---

## Scenario 5 – Ride Cancellation & Recovery Paths

Purpose: Validate failure and reversal flows for rides.

| Step | Request | Expected outcome |
| --- | --- | --- |
| 1 | `POST /rides/request` (new ride) | `201 Created` as in Scenario 4 Step 1. Note assigned driver ID for later validation.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L142-L200】 |
| 2 | `POST /rides/{{RIDE_ID}}/status` with `{ "status": "cancelled", "reason": "rider no-show" }` | `200 OK` showing status `cancelled`, reason retained, driver availability toggled back to `true`, Kafka `ride.status_changed` and RabbitMQ notifications emitted.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L230-L276】 |
| 3 | `POST /rides/{{RIDE_ID}}/status` with invalid `status` value | Response defaults to `cancelled` and returns `200 OK` with updated record; driver availability unchanged unless status set to `cancelled`/`completed`. Use to test validation leniency.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L204-L226】 |
| 4 | `POST /rides/unknown/status` | `404` error `Ride not found` with no side effects.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L202-L215】 |

---

## Scenario 6 – Location Insights & Cleanup

Purpose: Exercise read/nearby/delete APIs for the `LocationManager`.

| Step | Request | Expected outcome |
| --- | --- | --- |
| 1 | `GET /location/{{USER_ID}}` | `200 OK` returning `{ "location": {"user_id":..., "latitude":..., "longitude":..., "name":...}}` if stored; `404` when missing.【F:LocationManager/src/services/httpHandler/servers/httpLocationServer.cpp†L49-L60】【F:LocationManager/src/services/routeHandler/routeHandler.cpp†L155-L206】 |
| 2 | `GET /location/nearby?lat=37.7749&lng=-122.4194&radius=3&role=driver` | `200 OK` payload `{ "status":"success", "nearby":[{ "user_id":..., "distance_km":...}, ...] }` sorted by distance; radius sanitized to default when invalid input supplied.【F:LocationManager/src/services/httpHandler/servers/httpLocationServer.cpp†L62-L95】【F:LocationManager/src/services/routeHandler/routeHandler.cpp†L208-L280】 |
| 3 | `DELETE /location/{{USER_ID}}` | `200 OK` message `Location deleted`; `404` when record not found. Ensures cleanup for off-duty drivers.【F:LocationManager/src/services/httpHandler/servers/httpLocationServer.cpp†L97-L108】【F:LocationManager/src/services/routeHandler/routeHandler.cpp†L282-L344】 |

---

## Scenario 7 – Messaging & Observability Verification

Purpose: Confirm the system emits expected telemetry and background work after exercising previous scenarios.

1. **Kafka topics**
   - `ride_lifecycle_events`: should contain envelopes with `type` keys such as `ride.requested`, `ride.assigned`, and `ride.status_changed` originating from `RideManager` interactions.【F:RideManager/src/services/kafkaHandler/rideKafkaManager.cpp†L10-L47】
   - `<user events>` (topic name equals the event key) should contain `user_created`, `user_updated`, and `user_deleted` messages with nested `data` payloads when relevant UserManager mutations occur.【F:UserManager/src/services/kafkaHandler/producers/userKafkaManager.cpp†L10-L74】
2. **RabbitMQ queue `driver_notifications`** should receive JSON messages whenever rides are assigned or their statuses change, carrying `driver_id` and full ride payload for downstream dispatch workers.【F:RideManager/src/services/rabbitHandler/rideRabbitManager.cpp†L13-L47】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L174-L276】
3. **gRPC client logs** produced by `sharedResources::LocationClient::SendLocation` confirm that rider pickup coordinates were forwarded to the simulated LocationManager stream during ride requests.【F:sharedResources/src/sharedgRPCClienth.cpp†L6-L26】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L170-L201】
4. **UserManager `/health` endpoint** exposes DB latency, Kafka configuration, and environment metadata to verify readiness after workloads.【F:UserManager/src/services/routeHandler/routeHandler.cpp†L150-L198】

Use these verifications alongside service logs to confirm every asynchronous requirement and cross-service integration functions as designed.
