# syntax=docker/dockerfile:1.7

# Base on the upstream akk-stack image, which already provides the full
# EQEmu build environment (clang, cmake, ninja, ccache, custom eqemu-perl
# 5.32.1, lua5.1, libsodium/ssl/mysqlclient, mariadb-client, cron, ssh)
# plus the eqemu user, sudoers, logrotate, and the upstream entrypoint.pl.
#
# What we add: compile the engine and place artifacts under
# /home/eqemu/code/ at the paths upstream's create-symlinks.pl already
# expects (build/bin and utils/patches). The host-mounted /home/eqemu/server
# in akk-stack's docker-compose ends up with symlinks pointing into the
# image, so `docker compose pull && up -d` is a full engine upgrade.
FROM eqemulator/eqemu-server:v16

LABEL org.opencontainers.image.source="https://github.com/nightwreath/theo-and-co-engine"
LABEL org.opencontainers.image.description="Theo and Co — EQEmu engine fork (Phase 1+ camera/control patches)"
LABEL org.opencontainers.image.licenses="AGPL-3.0"

USER root
WORKDIR /build

ENV CC=/usr/bin/clang
ENV CXX=/usr/bin/clang++
ENV CCACHE_DIR=/root/.ccache

ARG GIT_SHA=unknown
ENV GIT_SHA=${GIT_SHA}

COPY . /build/

RUN --mount=type=cache,target=/root/.ccache \
    cmake -S . -B build -G Ninja \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCMAKE_C_COMPILER_LAUNCHER=ccache \
        -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
        -DEQEMU_BUILD_TESTS=OFF \
        -DEQEMU_BUILD_LOGIN=ON \
        -DEQEMU_BUILD_LUA=ON \
        -DEQEMU_BUILD_PERL=ON \
        -DEQEMU_BUILD_CLIENT_FILES=ON \
 && cmake --build build --parallel \
 && mkdir -p /home/eqemu/code/build/bin /home/eqemu/code/utils \
 && cp -af build/bin/. /home/eqemu/code/build/bin/ \
 && cp -af utils/patches /home/eqemu/code/utils/patches \
 && printf '%s\n' "${GIT_SHA}" > /home/eqemu/code/build-sha \
 && chown -R eqemu:eqemu /home/eqemu/code \
 && rm -rf /build

USER eqemu
WORKDIR /home/eqemu

# ENTRYPOINT inherited from base image:
#   ["/bin/bash", "-c", "entrypoint.pl; /bin/bash"]
# entrypoint.pl invokes ~/assets/scripts/create-symlinks.pl when
# /home/eqemu/server/eqemu_config.json exists; that script symlinks
# /home/eqemu/code/build/bin/<binary> into /home/eqemu/server/bin/<binary>
# and /home/eqemu/code/utils/patches/{*patch*,*opcodes*} into the
# server patches/opcodes dirs — both paths are baked above.
