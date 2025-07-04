#!/bin/bash
set -e

conan build . --build=missing -s build_type=Release

./build/Release/bin/UserManager
