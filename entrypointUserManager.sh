#!/bin/bash
set -euo pipefail

conan build . --output-folder=build --build=missing -s build_type=Release

./build/Release/bin/UserManager
