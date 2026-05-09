#!/bin/bash

set -ex

sudo chown eqemu:eqemu /drone/src/ * -R
sudo chown eqemu:eqemu /home/eqemu/.ccache/ * -R

chmod +x ./utils/scripts/build/source-db-build.sh
utils/scripts/build/source-db-build.sh &

git submodule init && git submodule update

perl utils/scripts/build/tag-version.pl

mkdir -p build && cd build && \
  cmake -DEQEMU_BUILD_TESTS=ON \
      -DCMAKE_BUILD_TYPE=Release \
      -DEQEMU_BUILD_STATIC=ON \
      -DEQEMU_BUILD_LOGIN=ON \
      -DEQEMU_BUILD_LUA=ON \
      -DEQEMU_BUILD_PERL=ON \
      -DCMAKE_CXX_FLAGS_RELEASE:STRING="-g -Wno-everything" \
      -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
      -G 'Unix Makefiles' \
      .. && make -j$((`nproc`-2))

curl https://raw.githubusercontent.com/Akkadius/eqemu-install-v2/master/eqemu_config.json --output eqemu_config.json
./bin/tests

ldd ./bin/zone

echo "Waiting for MariaDB to be ready..."
while ! mysqladmin ping -uroot -peqemu -hlocalhost --silent; do
    sleep 1
done

echo "# Cloning quests repository"
git -C ./quests pull 2> /dev/null || git clone https://github.com/ProjectEQ/projecteqquests.git quests

mkdir maps
mkdir logs

ln -s ./quests/lua_modules ./lua_modules
ln -s ./quests/plugins ./plugins

while [ ! -f /tmp/import-done ]; do
    sleep 1  # Wait for 1 second before checking again
done

echo "# Running world database updates"
FORCE_INTERACTIVE=1 ./bin/world database:updates --skip-backup --force

echo "# Running shared_memory"
./bin/shared_memory

echo "# Running NPC hand-in tests"
./bin/zone tests:npc-handins 2>&1 | tee test_output.log
./bin/zone tests:npc-handins-multiquest 2>&1 | tee -a test_output.log
./bin/zone tests:databuckets 2>&1 | tee -a test_output.log
./bin/zone tests:zone-state 2>&1 | tee -a test_output.log

if grep -E -q "QueryErr|Error|FAILED" test_output.log; then
    echo "Error found in test output! Failing build."
    exit 1
fi

# shellcheck disable=SC2164
cd /drone/src/

chmod +x ./utils/scripts/build/should-release/should-release
./utils/scripts/build/should-release/should-release || exit

rm ./build/bin/*.a
zip -j eqemu-server-linux-x64.zip ./build/bin/*

# shellcheck disable=SC2010
ls -lsh | grep zip
sudo apt-get update && sudo apt-get install -y rclone
rclone config create remote ftp env_auth true > /dev/null
rclone copy eqemu-server-linux-x64.zip remote: 2>&1
rclone ls remote: 2>&1
