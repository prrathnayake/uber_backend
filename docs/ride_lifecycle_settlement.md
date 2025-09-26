# Ride Booking to Driver Wallet Settlement – Execution Report

This report explains the full back-end flow from the moment a rider requests a trip until the driver receives the fare in their in-app wallet. Each step references the C++ services and database mutations that guarantee atomic ride lifecycle processing.

## 1. Rider submits a ride request

1. The `RideManager` HTTP layer validates the incoming JSON body (`rider_id`, pickup/dropoff details) and enriches it with optional `currency` and `payment_method` hints from the client.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L451-L521】
2. The service matches an available driver from cached availability (including a proximity query via the `LocationGateway`).【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L472-L505】
3. A new in-memory `RideRecord` is created, persisted to MySQL (including fare placeholders) via `persistRide`, and broadcast to Kafka (`ride.requested`, `ride.assigned`) and RabbitMQ for downstream dispatchers.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L507-L537】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L205-L261】
4. The shared `SingletonLogger` records every major action, ensuring observability without any `std::cout` statements across the project.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L323-L343】【F:RideManager/src/server.cpp†L38-L115】

## 2. Driver acknowledges and progresses the ride

1. Drivers (or automation) report their availability through `/drivers/{id}/status`. The handler snapshots the driver ledger and publishes Kafka events containing wallet balances for real-time dashboards.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L640-L664】
2. Status transitions (`accepted`, `in_progress`) update the ride record and persist the mutation, keeping database state and in-memory cache synchronized.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L558-L628】

## 3. Ride completion and fare settlement

1. When `/rides/{id}/status` is called with `status: completed` and a `fare`, the handler normalizes the fare value, stores it in the ride record, and marks the driver available again.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L569-L610】
2. `creditDriverWallet` increments both the driver’s current wallet balance and lifetime earnings under a thread-safe lock, logging the credit with the singleton logger for audit trails.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L323-L343】
3. `recordPaymentInDatabase` resolves the ride’s primary key, upserts the `payments` table with a completed `wallet` settlement, and reports success/failure through structured logs.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L352-L399】【F:RideManager/sql_scripts/database_init.sql†L34-L42】
4. The updated ride payload published to Kafka/RabbitMQ now carries a `payment` section (method + status) so consumers can verify the settlement asynchronously.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L99-L122】【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L630-L635】

## 4. Post-ride introspection and driver payouts

1. `handleGetDriverProfile` federates the profile data from `UserManager` and appends the latest wallet snapshot (balance and lifetime earnings), exposing the same ledger numbers that were updated during settlement.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L705-L718】
2. Operators can query `/drivers/{id}/status` to retrieve availability together with wallet totals, thanks to the enriched `driverState` payload described earlier.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L640-L664】
3. Every REST response and event stream originates from `rideToJson`, guaranteeing that downstream systems see the same fare and payment metadata that the wallet ledger used for settlement.【F:RideManager/src/services/routeHandler/rideRouteHandler.cpp†L99-L138】

## 5. Infrastructure required for test deployments

1. Build the shared toolchain image once (`Dockerfile.base`) and then spin up the dedicated Docker Compose stack that stands up Kafka, RabbitMQ, Redis, three MySQL instances, and all microservices from source.【F:docker/dockerCMD.txt†L1-L8】【F:docker/docker-compose.test-deploy.yml†L1-L126】
2. Each service container runs `conan build . --output-folder=build --build=missing -s build_type=Release` before launching, ensuring parity with local builds and exercising the wallet settlement flow under production-like conditions.【F:entrypointRideManager.sh†L1-L6】【F:entrypointUserManager.sh†L1-L6】【F:entrypointLocationManager.sh†L1-L6】

With these components working in concert, the rider’s fare is deterministically routed from request validation through driver payout without manual intervention, and all observability tooling consumes a consistent payment record.
