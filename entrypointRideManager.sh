#!/bin/bash
set -euo pipefail

conan build . --build=missing -s build_type=Release

./build/Release/bin/RideManager
