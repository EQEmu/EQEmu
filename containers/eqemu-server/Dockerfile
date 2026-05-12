# syntax=docker/dockerfile:1.7

# Base on the upstream akk-stack image, which already provides the full
# EQEmu build environment (clang, cmake, ninja, ccache, custom eqemu-perl
# 5.32.1, lua5.1, libsodium/ssl/mysqlclient, mariadb-client, cron, ssh).
# We add the compile step + place binaries at /opt/eqemu/bin, and wrap the
# upstream entrypoint with a binary-sync step that copies them into
# /home/eqemu/server/bin/ (which is bind-mounted from the host in the
# stack's docker-compose).
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
 && mkdir -p /opt/eqemu/bin \
 && cp -af build/bin/. /opt/eqemu/bin/ \
 && printf '%s\n' "${GIT_SHA}" > /opt/eqemu/build-sha \
 && chown -R eqemu:eqemu /opt/eqemu

COPY --chown=eqemu:eqemu containers/eqemu-server/sync-binaries.sh /bin/sync-binaries.sh
RUN chmod +x /bin/sync-binaries.sh

# Slim the image — the source and build tree are no longer needed.
RUN rm -rf /build

USER eqemu
WORKDIR /home/eqemu

# Wrap the upstream entrypoint with our sync step so the host-mounted
# /home/eqemu/server/bin gets updated to whatever this image was built with.
ENTRYPOINT ["/bin/bash", "-c", "/bin/sync-binaries.sh && entrypoint.pl; /bin/bash"]
