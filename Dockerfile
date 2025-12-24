FROM debian:testing-slim

ENV DEBIAN_FRONTEND=noninteractive

ARG USER_ID=1000
ARG GROUP_ID=1000
ARG USER_NAME=trinity

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    git \
    curl \
    ca-certificates \
    cmake \
    ninja-build \
    ccache \
    qt6-base-dev \
    qt6-base-dev-tools \
    qt6-declarative-dev \
    qt6-tools-dev \
    qt6-l10n-tools \
    libgl1-mesa-dev \
    libvulkan-dev \
    && apt-get clean && rm -rf /var/lib/apt/lists/*

ENV PATH="/usr/lib/ccache:$PATH"

RUN groupadd -g ${GROUP_ID} ${USER_NAME} || true && \
    useradd -l -u ${USER_ID} -g ${GROUP_ID} -m ${USER_NAME} || true && \
    install -d -m 0755 -o ${USER_NAME} -g ${GROUP_ID} /project

USER ${USER_NAME}
WORKDIR /project

ENV CMAKE_GENERATOR="Ninja"

CMD ["/bin/bash"]
