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

## Scenario 8 – Ride Request Cross-Service Debug Trace

Purpose: Observe every subsystem a single happy-path ride request touches, together with the console breadcrumbs emitted by each component.

| Step | Trigger | Expected payload & storage | Console / observability checkpoints |
| --- | --- | --- | --- |
| 1 | Start `RideManager` binary | Loads HTTP handlers and background consumers. | Look for `INFO` log `Initialising RideManager HTTP handlers` from `RideServer::createHttpServers` followed by Kafka/Rabbit bootstrap messages when consumers attach.【F:RideManager/src/server.cpp†L23-L70】 |
| 2 | `POST /drivers/{{DRIVER_ID}}/status` → `{ "available": true }` | Driver marked available in memory, Kafka `driver.status_changed` event published. | Console prints remain silent on success; monitor Kafka topic to confirm envelope `{"type":"driver.status_changed",...}` as described earlier.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L298-L332】 |
| 3 | `POST /rides/request` with rider and coordinates | Ride persisted, driver reserved, Kafka `ride.requested`/`ride.assigned` and Rabbit driver notification emitted. | `RideManager` Kafka consumer echoes `[Kafka][RideManager] lifecycle event -> {...}` for each event, while Rabbit consumer prints `[RabbitMQ][RideManager] driver task -> {...}` when notifications loop back through the shared handler.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L173-L217】【F:RideManager/src/server.cpp†L41-L64】 |
| 4 | Same request triggers gRPC bridge | Pickup lat/lng forwarded to Location stream. | Enable debug logs for the gRPC client; the helper prints `Streaming location update to LocationManager host:port` before delivering payloads.【F:sharedResources/src/sharedgRPCClienth.cpp†L6-L26】 |
| 5 | Location service receives driver location | Fetches nearby drivers via HTTP + optional Kafka backplane. | Monitor LocationManager consumer console lines such as `[Kafka Msg] {...}` when new location events arrive and `[RabbitMQ] received payload: {...}` for queue callbacks.【F:LocationManager/src/server.cpp†L45-L71】 |
| 6 | `POST /rides/{{RIDE_ID}}/status` transitions (`accepted`, `in_progress`, `completed`) | Updates in-memory state, database rows, publishes `ride.status_changed` each time; completion frees driver. | Ride consumer prints each Kafka message; RabbitMQ consumer shows final dispatch message; DB persistence occurs via `persistRide` mutation.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L218-L276】【F:RideManager/src/server.cpp†L41-L64】 |
| 7 | `GET /rides/{{RIDE_ID}}` & `/drivers/{{DRIVER_ID}}/profile` | Confirms final record and cross-service profile fetch from `UserManager`. | When profile lookup hits `/user/{{DRIVER_ID}}`, inspect `UserManager` console `[Kafka][UserManager] profile update event -> ...` if upstream modifications occurred; HTTP responses confirm federation.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L331-L386】【F:UserManager/src/server.cpp†L27-L67】 |

Console checkpoints make it easy to pinpoint regressions: missing Kafka echo hints at producer/consumer misconfiguration, absent Rabbit message indicates queue or binding issues, and a missing profile response highlights inter-service HTTP or authentication problems.

---

## Scenario 9 – User Lifecycle Messaging Deep Dive

Purpose: Validate that every user mutation publishes the correct Kafka envelope and optional RabbitMQ task while confirming HTTP contracts.

| Step | Trigger | Expected messaging side effects | Console checkpoints |
| --- | --- | --- | --- |
| 1 | `POST /signup` | Kafka `user_created` event containing profile JSON and subject ID.【F:UserManager/src/services/kafkaHandler/producers/userKafkaManager.cpp†L18-L48】 | Start `UserManager` Kafka consumer to see `[Kafka][UserManager] profile update event -> {...}` for each emission.【F:UserManager/src/server.cpp†L27-L53】 |
| 2 | `PUT /user/{{USER_ID}}` | Kafka `user_updated` event with delta; repeated fields overwritten.【F:UserManager/src/services/kafkaHandler/producers/userKafkaManager.cpp†L50-L64】 | Consumer prints updated payload; compare with DB row if needed.【F:UserManager/src/server.cpp†L27-L53】 |
| 3 | `DELETE /user/{{USER_ID}}` | Kafka `user_deleted` event, payload contains identifier only.【F:UserManager/src/services/kafkaHandler/producers/userKafkaManager.cpp†L66-L74】 | Console log includes deletion envelope, verifying tombstone semantics.【F:UserManager/src/server.cpp†L27-L53】 |
| 4 | Optional: enable RabbitMQ dispatcher | When extended tasks are configured, queue callback prints `[RabbitMQ][UserManager] task received -> {...}` for asynchronous onboarding/back-office duties.【F:UserManager/src/server.cpp†L55-L67】 |

Use this scenario after running onboarding flows to make sure user topics contain an auditable history for downstream analytics and compliance processes.

---

## Scenario 10 – Location Manager Cache & Redis Harness

Purpose: Exercise the in-memory Redis abstraction that LocationManager uses for proximity lookups so you can validate caching independently of HTTP flows.

| Step | Action | Expected outcome |
| --- | --- | --- |
| 1 | Launch `LocationManager` binary | Constructor logs `Initialised in-memory Redis handler`, signalling the stub cache is active.【F:LocationManager/src/services/redisHandler/redisHandler.cpp†L6-L15】 |
| 2 | Compile a lightweight diagnostic (example below) linking against `LocationManager` to call `RedisHandler::setValue`/`getValue`. | `getValue` returns stored payload, confirming key-value semantics; `addDriverToSet`/`getDriversFromSet` maintain H3-indexed availability lists.【F:LocationManager/src/services/redisHandler/redisHandler.cpp†L18-L43】 |
| 3 | Integrate with HTTP flow (optional) by inserting cache hooks in `/location/update` to persist the latest driver H3 index before recompiling. | On startup, observe the same constructor log; after HTTP updates, `getDriversFromSet` should be populated when invoked from your added instrumentation. |

Minimal diagnostic snippet:

```cpp
#include "LocationManager/include/services/redisHandler/redisHandler.h"

int main() {
    UberBackend::RedisHandler cache;
    cache.setValue("driver:501", "{\"lat\":37.7749,\"lng\":-122.4194}");
    auto payload = cache.getValue("driver:501");
    cache.addDriverToSet("8c2a9472b", "501");
    auto drivers = cache.getDriversFromSet("8c2a9472b");
}
```

Running the binary under `gdb` or adding temporary `std::cout` statements lets you step through cache mutations exactly like a production Redis round-trip while maintaining deterministic local testing.

---

## Scenario 11 – Failure Injection & Error Surface Mapping

Purpose: Deliberately disable dependencies to confirm graceful degradation and pinpoint failure domains.

| Step | Failure mode | Expected API result | Diagnostics |
| --- | --- | --- | --- |
| 1 | Stop Kafka broker before calling `POST /rides/request`. | HTTP still returns `201` because producer short-circuits when `createProducer` fails; rides persist but no lifecycle events publish.【F:RideManager/src/services/kafkaHandler/rideKafkaManager.cpp†L15-L33】 | Startup log warns `Failed to create Kafka producer for ride events`; consumer echoes cease, signalling event loss.【F:RideManager/src/services/kafkaHandler/rideKafkaManager.cpp†L20-L27】【F:RideManager/src/server.cpp†L41-L64】 |
| 2 | Stop RabbitMQ before ride request. | Ride creation succeeds but driver notifications are skipped when producer missing.【F:RideManager/src/services/rabbitHandler/rideRabbitManager.cpp†L16-L46】 | Warning `Failed to create RabbitMQ producer for driver notifications` appears; queue consumers emit nothing.【F:RideManager/src/services/rabbitHandler/rideRabbitManager.cpp†L24-L31】【F:RideManager/src/server.cpp†L55-L71】 |
| 3 | Kill UserManager before fetching driver profile. | `GET /drivers/{{DRIVER_ID}}/profile` returns `404` with `Driver not found` because cross-service call fails and response is discarded.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L318-L386】 | Ride server logs absence indirectly; use `curl` against UserManager health check to confirm outage.【F:UserManager/src/services/routeHandler/routeHandler.cpp†L150-L198】 |
| 4 | Provide malformed ride status payload. | API replies `400` `Missing status field` when `status` absent; existing record untouched.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L218-L236】 | HTTP error body pinpoints validation guard, aiding regression debugging. |

Capturing these degradation behaviors upfront ensures the playbook doubles as an incident response reference when dependencies fail in production.

