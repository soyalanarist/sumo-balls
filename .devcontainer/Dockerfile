# Devcontainer image for sumo-balls
FROM mcr.microsoft.com/devcontainers/cpp:ubuntu-22.04

# Install runtime/build dependencies
RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y \
        libsfml-dev \
        libgl1-mesa-dev \
    && apt-get clean \
    && rm -rf /var/lib/apt/lists/*
