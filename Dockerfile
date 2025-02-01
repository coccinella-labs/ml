# Multi-stage build for efficient image
FROM ubuntu:20.04 AS builder

# Avoid prompts from apt
ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    libopenmpi-dev \
    libopencv-dev \
    libeigen3-dev \
    libcpprest-dev \
    libboost-all-dev \
    git \
    wget

# Set working directory
WORKDIR /app

# Copy project files
COPY . .

# Build the project
RUN mkdir build && cd build && \
    cmake .. && \
    make

# Final stage
FROM ubuntu:20.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    libopenmpi3 \
    libopencv4.2-dev \
    libeigen3-dev \
    libcpprest2.10 \
    && rm -rf /var/lib/apt/lists/*

# Copy built binary from builder stage
COPY --from=builder /app/build/distributed_ml_app /usr/local/bin/

# Expose dashboard port
EXPOSE 8080

# Default command
CMD ["distributed_ml_app"]
