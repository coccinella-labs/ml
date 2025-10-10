# multi-stage build for efficient image
FROM ubuntu:20.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential cmake libopenmpi-dev \
    libeigen3-dev libcpprest-dev libboost-all-dev git wget \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app
COPY . .

RUN mkdir build && cd build && cmake .. && make

FROM ubuntu:20.04

RUN apt-get update && apt-get install -y \
    libopenmpi3 libeigen3-dev libcpprest2.10 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=builder /app/build/distributed_ml_app /usr/local/bin/

EXPOSE 8080
CMD ["distributed_ml_app"]
