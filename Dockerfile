# ---- Builder stage ----
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /app

# Essential build deps only
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake git ca-certificates wget python3 \
    libopenmpi-dev libeigen3-dev libcpprest-dev libboost-all-dev \
    nlohmann-json3-dev libssl-dev pkg-config \
    && rm -rf /var/lib/apt/lists/*

COPY . /app
RUN cmake -B build -S . -DCMAKE_BUILD_TYPE=Release \
 && cmake --build build -j$(nproc) --config Release

# ---- Runtime stage ----
FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
WORKDIR /app
RUN useradd -m appuser && apt-get update && apt-get install -y --no-install-recommends \
    libopenmpi3 libcpprest2.10 libssl3 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/distributed_ml /usr/local/bin/distributed_ml
USER appuser
EXPOSE 8080
ENTRYPOINT ["distributed_ml"]

LABEL org.opencontainers.image.source="https://github.com/bniladridas/ml" \
      org.opencontainers.image.description="Distributed ML framework (ARM64 optimized)"
