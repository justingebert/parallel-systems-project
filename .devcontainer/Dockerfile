FROM ubuntu:24.04

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    libopenmpi-dev \
    openmpi-bin \
    gdb \
    git \
    ca-certificates \
    openssh-client \
    python3 \
    python3-matplotlib \
    && rm -rf /var/lib/apt/lists/*

USER ubuntu
WORKDIR /workspace
