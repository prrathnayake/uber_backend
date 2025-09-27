# Uber Backend Code Review

This document captures a high-level review of the repository with an eye on reusing the shared functionality that lives in the `ccp-base` library.  The focus areas are code reuse, deployment ergonomics, and build reproducibility.

## Summary of Findings

1. **Shared utilities are already centralised** – The `sharedUtils` and `sharedResources` targets expose logging, configuration, HTTP, Kafka, RabbitMQ, gRPC, and persistence helpers.  These modules should become thin wrappers over the corresponding primitives in `ccp-base` as that project already provides canonical implementations.  Where possible, replace bespoke helpers (for example the JWT helper or SHA-256 utility) with the `ccp-base` version to avoid diverging behaviour.
2. **Duplicate sources** – The `UserManager` service still contained an empty `bankdetails.cpp` translation unit in addition to the real `bankDetails.cpp`.  Removing the duplicate eliminates unnecessary compilation work and prevents accidental symbol clashes.
3. **Consistent Conan + CMake workflow** – Every service entrypoint already builds from source with `conan build . --output-folder=build --build=missing`.  After installing Conan 2 and running `conan profile detect`, a clean build succeeds with GCC 13.3 on Ubuntu 22.04.  Any Docker environment that aims to test-deploy the stack should replicate these exact steps.
4. **Docker test deployment** – The `docker/docker-compose.test-deploy.yml` file provisions MySQL, Kafka, RabbitMQ, and Redis and uses service-specific Dockerfiles that run the Conan build inside each container before launching the binary.  This environment is suitable for validating the microservices end-to-end; ensure that the base image also installs the `ccp-base` artefacts so that the services can link against them during the build phase.
5. **Next steps for `ccp-base` adoption** – Promote the reusable pieces (`sharedUtils`, `sharedResources`, and related models) into Conan packages emitted by `ccp-base`.  Once they are published, the per-service CMake targets here can consume them via `find_package` instead of compiling local copies.  This migration will shrink the repository, reduce duplicated maintenance, and make future services easier to bootstrap.

## Recommended Actions

- Publish the relevant `ccp-base` components as Conan packages and update this repository's `conanfile.py` to depend on them.  Start with core utilities (logging, configuration, HTTP clients/servers) and database connectors.
- Replace custom cryptography and JWT helpers in `sharedUtils` with the vetted equivalents from `ccp-base`.  Doing so will ensure consistent token semantics across projects.
- Remove any residual duplicated or stub `.cpp` files (such as the deleted `bankdetails.cpp`) and tighten the CMake source globbing to avoid picking up unintended files.
- Extend the Docker base image to install or mount the `ccp-base` artefacts so the containerised builds automatically reuse the shared library.
- Add automated CI (GitHub Actions or similar) that runs `conan build` and the container orchestration to catch regressions early and validate that the services run cleanly in a production-like environment.

