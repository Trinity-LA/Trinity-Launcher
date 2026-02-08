FROM debian:bookworm-slim

ARG UID=1000
ARG GID=1000

ENV DEBIAN_FRONTEND=noninteractive \
  PATH="/home/trinity/.cargo/bin:${PATH}" \
  QT_QPA_PLATFORM=xcb

RUN --mount=type=cache,target=/var/cache/apt \
  --mount=type=cache,target=/var/lib/apt \
  apt-get update && apt-get install -y --no-install-recommends \
  build-essential git curl cmake clang ninja-build \
  qt6-base-dev qt6-base-dev-tools qt6-declarative-dev qt6-webengine-dev qt6-svg-dev qt6-tools-dev \
  libcurl4-openssl-dev libssl-dev \
  libasound2-dev libpulse-dev libjack-jackd2-dev libpipewire-0.3-dev libsndio-dev \
  libx11-dev libxi-dev libxext-dev libxfixes-dev libxcursor-dev libxrandr-dev libxss-dev libxtst-dev \
  libgl1-mesa-dev libegl1-mesa-dev libgles2-mesa-dev libvulkan-dev vulkan-validationlayers \
  libdrm-dev libgbm-dev \
  libudev-dev libevdev-dev libusb-1.0-0-dev libdbus-1-dev libbluetooth-dev bluez \
  libibus-1.0-dev libxkbcommon-dev \
  libpng-dev libzip-dev libcups2-dev \
  libwayland-dev libunwind-dev libdecor-0-dev sudo && \
  rm -f /etc/apt/apt.conf.d/docker-clean

RUN groupadd -g "${GID}" trinity && \
  useradd -l -u "${UID}" -g trinity -m -s /bin/bash trinity && \
  echo "trinity ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers && \
  mkdir -p /tmp/runtime-trinity && \
  chmod 0700 /tmp/runtime-trinity && \
  chown trinity:trinity /tmp/runtime-trinity

USER trinity
WORKDIR /home/trinity/project

RUN curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y
RUN git config --global --add safe.directory /project

CMD ["/bin/bash"]
