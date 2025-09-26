# Automation Test Overview

- Build executed with `conan build . --output-folder=build --build=missing`, producing UserManager, RideManager, and LocationManager binaries.
- Docker CLI is unavailable in the environment (`docker --version` is not installed), so containerized dependencies (Kafka, RabbitMQ, databases) could not be provisioned.
- Runtime validation and the scenarios defined in `docs/system_validation_scenarios.md` remain unexecuted because the required infrastructure could not be started.
- Generated `test_overview_report.pdf` summarizing the build status and blockers.
