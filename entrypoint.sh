#!/bin/bash
set -e

conan profile detect

git clone https://github.com/prrathnayake/cpp-base.git
git clone https://github.com/prrathnayake/cpp-tools.git

conan build ./cpp-tools --build=missing

conan export ./cpp-tools --user=pasan --channel=testing

python3 ./cpp-tools/module/scripts/buildBase.py ./cpp-base/

conan build . --build=missing -s build_type=Release

./build/Release/bin/UserManager
