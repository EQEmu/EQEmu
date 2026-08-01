# Build binaries: make cmake, make build
# One time initial setup (or to reset db): make prep, make inject-mariadb, make maps
# Update custom db edits: make inject-custom
# Start up server: make shared, make login, make world, make zone
# in game, stop combat spam #logs set gmsay 79 0
# in game, stop loot spam #logs set gmsay 69 0
NAME := eqemu-server

# mkdir -p /tmp/vcpkg-buildtrees
# mv submodules/vcpkg/buildtrees/* /tmp/vcpkg-buildtrees/
# rm -rf submodules/vcpkg/buildtrees
# ln -s /tmp/vcpkg-buildtrees submodules/vcpkg/buildtrees
.PHONY: cmake
cmake: is-vscode
	git submodule update --init --recursive
	cmake -S . -B build -G Ninja \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_C_COMPILER_LAUNCHER=ccache \
		-DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
		-DCMAKE_HAVE_LIBC_PTHREAD=1 \
		-DEQEMU_BUILD_TESTS=ON \
		-DEQEMU_BUILD_LOGIN=ON \
		-DEQEMU_BUILD_LUA=ON \
		-DEQEMU_BUILD_PERL=ON \
		-DEQEMU_BUILD_CLIENT_FILES=OFF

.PHONY: build
build: is-vscode
	cmake --build build --parallel 4

.PHONY: prep
prep: is-vscode
	@echo "Preparing build/bin for usage..."

	@echo "Prepping folders..."
	@mkdir -p .devcontainer/override
	@mkdir -p .devcontainer/repo
	@mkdir -p .devcontainer/cache
	@mkdir -p build/bin/logs
	@mkdir -p build/bin/shared
	@mkdir -p build/bin/assets

	@echo "Applying overrides..."
	@if [ ! -f .devcontainer/override/eqemu_config.json ]; then cp .devcontainer/base/eqemu_config.json .devcontainer/override/eqemu_config.json; fi
	@if [ -f build/bin/eqemu_config.json ]; then unlink build/bin/eqemu_config.json; fi
	cd build/bin && ln -s ../../.devcontainer/override/eqemu_config.json eqemu_config.json
	@if [ ! -f .devcontainer/override/login.json ]; then cp .devcontainer/base/login.json .devcontainer/override/login.json; fi
	@if [ -f build/bin/login.json ]; then unlink build/bin/login.json; fi
	cd build/bin && ln -s ../../.devcontainer/override/login.json login.json

	@echo "Cloning repositories..."
	cd .devcontainer/repo && if [ ! -d "quests" ]; then cd ../../.devcontainer/repo/ && git clone https://github.com/ProjectEQ/projecteqquests.git quests; fi
	cd .devcontainer/repo && if [ ! -d "eqemu-definitions" ]; then cd ../../.devcontainer/repo/ && git clone https://github.com/xackery/eqemu-definitions.git eqemu-definitions; fi
	cd .devcontainer/repo && if [ ! -d "maps" ]; then cd ../../ && make maps; fi
	cd .devcontainer/repo && if [ ! -d "eq-core-dll" ]; then cd ../../.devcontainer/repo/ && git clone https://github.com/xackery/eq-core-dll.git eq-core-dll; fi

	@echo "Applying AoTv4 custom overlay (tracked source over the bootstrapped clones)..."
	cp -R .devcontainer/custom/quests/. .devcontainer/repo/quests/
	cp -R .devcontainer/custom/eq-core-dll/. .devcontainer/repo/eq-core-dll/

	@if [ -d build/bin/quests ]; then unlink build/bin/quests; fi
	cd build/bin && ln -s ../../.devcontainer/repo/quests quests
	@if [ -d build/bin/maps ]; then unlink build/bin/maps; fi
	cd build/bin && ln -s ../../.devcontainer/repo/maps maps
	@if [ -d build/bin/eqemu-definitions ]; then unlink build/bin/eqemu-definitions; fi
	cd build/bin && ln -s ../../.devcontainer/repo/eqemu-definitions eqemu-definitions
	@mkdir -p build/bin/quests/mods

	@echo "Applying base links..."
	cp -R -u -p utils/patches .devcontainer/base/
	@if [ -d build/bin/assets/patches ]; then unlink build/bin/assets/patches; fi
	cd build/bin/assets && ln -s ../../../.devcontainer/base/patches patches
	@if [ -d build/bin/lua_modules ]; then unlink build/bin/lua_modules; fi
	cd build/bin && ln -s ../../.devcontainer/repo/quests/lua_modules lua_modules
	@if [ -d build/bin/mods ]; then unlink build/bin/mods; fi
	cd build/bin && ln -s ../../.devcontainer/repo/quests/mods mods
	@if [ -d build/bin/plugins ]; then unlink build/bin/plugins; fi
	cd build/bin && ln -s ../../.devcontainer/repo/quests/plugins plugins

	@echo "Eqemu is prepared. Edit build/bin/eqemu_config.json to configure."

# Refresh the tracked .devcontainer/custom overlay from the LIVE repo/ files. Run this before
# committing so the version-controlled copies match what you've been editing/testing in repo/.
.PHONY: sync-custom
sync-custom:
	@echo "Syncing live repo customizations -> tracked .devcontainer/custom overlay..."
#
# ⚠️⚠️ THIS USED TO BE A HARDCODED LIST OF 8 FILES AND IT SILENTLY LOST WORK.
# `.devcontainer/repo/` is gitignored (.devcontainer/.gitignore), so the ONLY copy of the Lua and the
# dll source that reaches the `aotv4-spell-rebuild` branch is this overlay. On 2026-08-01 the live
# quests clone had 65 changed files against 8 the list copied -- global_npc.lua, every aotv4_*
# module, lua_modules/commands/, the resplendent NPCs, the 43xxx spell scripts and two edited stock
# files were ALL outside it. Anything not copied here is invisible to git and dies with the container.
#
# ⚠️⚠️ AND `cp` CANNOT EXPRESS A DELETION. The overlay was still carrying the pre-renumber
# global/spells/50*.lua scripts (see section 14 -- the set moved 50xxx -> 43xxx long ago), which
# `make inject-custom` would have copied straight back over a fresh clone, resurrecting 16 dead
# scripts. `rsync --delete` on the directories we own is what fixes that, so a file removed from the
# clone is removed from the overlay too.
#
# 📌 ASK GIT WHAT IS OURS -- do not curate a list, and do not mirror whole directories either.
# The clone knows exactly what we have changed: everything between the ProjectEQ merge-base and now,
# committed or not. That is 82 files, where mirroring lua_modules/ + global/ + resplendent/ would
# drag in 285 -- ~200 of them untouched upstream scripts that have no business in this repo.
# ⚠️ It is also MORE complete than a directory list, not less: it picks up the stock quests we have
# edited in odd corners (tutorialb, soldungb, permafrost, oasis, delveb, fearplane, plugins) that
# nobody would have remembered to add. That is the failure mode this replaces.
# ⚠️ Deletions are handled both ways -- a file we removed from the clone is removed from the overlay,
# and any overlay file no longer in the list is pruned. That is what finally cleared the pre-renumber
# global/spells/50*.lua scripts, which `cp` had been faithfully re-injecting over every fresh clone.
	@set -e; \
	Q=.devcontainer/repo/quests; O=.devcontainer/custom/quests; \
	BASE=$$(git -C $$Q merge-base origin/master HEAD 2>/dev/null); \
	if [ -z "$$BASE" ]; then \
	  echo "ERROR: no ProjectEQ merge-base. Run: git -C $$Q fetch origin"; exit 1; fi; \
	L=$$(mktemp); \
	{ git -C $$Q diff --name-only $$BASE HEAD; \
	  git -C $$Q status --porcelain -uall | awk '{print $$NF}'; } | sort -u > $$L; \
	( cd $$O && find . -type f | sed 's|^\./||' ) | while read -r f; do \
	  grep -qxF "$$f" $$L || rm -f "$$O/$$f"; \
	done; \
	while read -r f; do \
	  if [ -f "$$Q/$$f" ]; then mkdir -p "$$O/$$(dirname $$f)"; cp -p "$$Q/$$f" "$$O/$$f"; \
	  else rm -f "$$O/$$f"; fi; \
	done < $$L; \
	find $$O -type d -empty -delete 2>/dev/null || true; \
	echo "  quests: $$(wc -l < $$L) files are ours (vs ProjectEQ $$BASE)"; \
	rm -f $$L
#
# ⚠️ THE dinput8.dll SOURCE IS DELIBERATELY NOT SYNCED HERE. The client dll and the UI files reach
# players through the CLIENT PATCHER, not through this repo, and the dll has its own repo and branch
# (Twochords/Dinput, aotv4-advloot-module -- see CLAUDE.md section 0). This target exists to make the
# SERVER side pushable from `aotv4-spell-rebuild`; mirroring 66 .cpp files of client code into it
# only creates a second, diverging copy of something that is already version-controlled elsewhere.
# 📌 The handful of dll files still tracked under .devcontainer/custom/eq-core-dll/ predate this and
# are left alone rather than churned; treat the dll repo as authoritative.
	@echo "Synced (server side: quests Lua only). Review with: git status .devcontainer/custom"

is-vscode:
	@if [ -z "$$REMOTE_CONTAINERS" ]; then \
		echo "Not running in VS Code devcontainer"; \
		exit 1; \
	fi

clean: is-vscode
	rm -rf build

.PHONY: maps
maps: is-vscode
	@echo "Downloading maps..."
	@mkdir -p .devcontainer/repo/maps
	@cd .devcontainer/repo/maps && wget -nc https://github.com/EQEmu/maps/archive/refs/heads/master.zip
	@cd .devcontainer/repo/maps && unzip -o master.zip
	@cd .devcontainer/repo/maps && mv maps-master/* .
	@cd .devcontainer/repo/maps && rm -rf maps-master
	@echo "Maps downloaded."

# Runs tests
.PHONY: test
test: is-vscode
	cd build/bin && ./tests

# Runs login binary
.PHONY: login
login: is-vscode check-mariadb
	cd build/bin && ./loginserver

.PHONY: hotfix
hotfix: shared

# Runs shared_memory binary
.PHONY: shared
shared: is-vscode check-mariadb
	cd build/bin && ./shared_memory

# Runs zone binary
.PHONY: zone
zone: is-vscode check-mariadb
	@find build/bin/logs/zone/ -type f -name 'zone*.log' -exec rm -f {} +
	cd build/bin && ./zone

check-mariadb: is-vscode
	@if ! sudo service mariadb status | grep -q 'active (running)'; then \
		sudo service mariadb start; \
	fi

# Runs world binary
.PHONY: world
world: is-vscode check-mariadb
	@find build/bin/logs/ -type f -name 'world*.log' -exec rm -f {} +
	cd build/bin && ./world

# Runs ucs binary
.PHONY: ucs
ucs: is-vscode check-mariadb
	@find build/bin/logs/ -type f -name 'ucs*.log' -exec rm -f {} +
	cd build/bin && ./ucs

# Runs queryserv binary
.PHONY: queryserv
queryserv: is-vscode check-mariadb
	@find build/bin/logs/ -type f -name 'query_server*.log' -exec rm -f {} +
	cd build/bin && ./queryserv

reset-content:
	@echo "Resetting content tables in database peq..."
	cd .devcontainer/cache/db/peq-dump && sudo mariadb --database peq -e "source create_tables_content.sql"

valgrind-%: is-vscode
	cd build/bin && valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --verbose --log-file=logs/$*.valgrind.log ./$*

# Start mariaDB standalone
.PHONY: mariadb
mariadb:
	@sudo service mariadb start

.PHONY: inject-mariadb
inject-mariadb:
	-sudo service mariadb start
	-mkdir -p .devcontainer/cache/db/
	-sudo mariadb -e 'DROP DATABASE IF EXISTS peq;'
	-sudo mariadb -e 'CREATE DATABASE peq;'
	-sudo mariadb -e "CREATE USER IF NOT EXISTS 'peq'@'127.0.0.1' IDENTIFIED BY 'peqpass';"
	-sudo mariadb -e "GRANT ALL PRIVILEGES ON *.* TO 'peq'@'127.0.0.1';"
ifeq (,$(wildcard .devcontainer/cache/db/db.sql.zip))
	@echo ".devcontainer/cache/db.sql.zip not found. Downloading database from https://db.eqemu.dev/latest"
	wget -nc https://db.eqemu.dev/latest -O .devcontainer/cache/db/db.sql.zip
	-cd .devcontainer/cache/db && unzip db.sql.zip
endif
	@echo "Sourcing db may take a while, please wait..."

	@cd .devcontainer/cache/db/peq-dump && sudo mariadb --database peq -e "source create_tables_content.sql"
	@cd .devcontainer/cache/db/peq-dump && sudo mariadb --database peq -e "source create_tables_login.sql"
	@cd .devcontainer/cache/db/peq-dump && sudo mariadb --database peq -e "source create_tables_player.sql"
	@# deprecated cd .devcontainer/cache/db/peq-dump && sudo mariadb --database peq -e "source create_tables_queryserv.sql"
	@cd .devcontainer/cache/db/peq-dump && sudo mariadb --database peq -e "source create_tables_state.sql"
	@cd .devcontainer/cache/db/peq-dump && sudo mariadb --database peq -e "source create_tables_system.sql"

	@echo "MariaDB is now injected."

.PHONY: gm-%
gm-%: is-vscode
	sudo mariadb --database peq -e "UPDATE account SET status=255 WHERE name = '$*';"
	@echo "Account $* is now a GM. /camp to have it go into effect."

depends: is-vscode
	sudo apt install graphviz pip time
	pip3 install graphviz
	mkdir -p build/depends
	@if [ ! -f "build/depends/dependency_graph.py" ]; then \
		wget https://raw.githubusercontent.com/pvigier/dependency-graph/master/dependency_graph.py -O build/depends/dependency_graph.py; \
	fi
	@echo "Generating dependency graphs (This may take a while)..."
	@echo "Login..."
	time python3 build/depends/dependency_graph.py -f png login build/depends/login.dot
	@echo "World..."
	time python3 build/depends/dependency_graph.py -f png world build/depends/world.dot
	@echo "Zone..."
	time python3 build/depends/dependency_graph.py -f png zone build/depends/zone.dot
	@echo "Common..."
	time python3 build/depends/dependency_graph.py -f png common build/depends/common.dot

backup: is-vscode
	@mkdir -p build/bin/backup
	cd build/bin && ./world database:dump --compress --player-tables --state-tables --system-tables --query-serv-tables

restore-%: is-vscode
	@if [ -z "$*" ]; then \
		echo "Please provide a backup file to restore from. Example: make restore-backup.sql"; \
		exit 1; \
	fi
	@echo "Restoring from backup $*"
	@sudo mariadb --database peq -e "$*"


cpu-zone: is-vscode
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@echo "This makefile is not intended to be run from the .devcontainer directory."
	exit
endif
	@cd build/bin && mkdir -p tmp
	cd build/bin && CPUPROFILE=prof.out ./zone

pprof-zone: is-vscode
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@echo "This makefile is not intended to be run from the .devcontainer directory."
	exit
endif
	cd build/bin && google-pprof --pdf zone prof.out > prof.pdf

pprof-gv-zone: is-vscode
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@echo "This makefile is not intended to be run from the .devcontainer directory."
	exit
endif
	cd build/bin && google-pprof --gv zone prof.out > prof.gv

heap-zone: is-vscode
ifeq ($(findstring .devcontainer,$(CURDIR)),.devcontainer)
	@echo "This makefile is not intended to be run from the .devcontainer directory."
	exit
endif
	@cd build/bin && mkdir -p tmp
	cd build/bin && HEAPPROFILE=prof.out ./zone


.PHONY: pull
pull:
	git pull
	@if [ ! -d "quests" ]; then git clone https://github.com/rebuildeq/quests.git quests; fi
	cd quests && git pull
	@if [ ! -d "eqemu-definitions" ]; then git clone https://github.com/xackery/eqemu-definitions.git eqemu-definitions; fi
	cd eqemu-definitions && git pull