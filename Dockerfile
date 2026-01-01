FROM debian:testing

RUN echo 'APT::Get::Assume-Yes "true";' > /etc/apt/apt.conf.d/90assumeyes

RUN apt-get update && \
    apt-get install -y \
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
    libwayland-dev libunwind-dev libdecor-0-dev \
    && curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh -s -- -y \
    && rm -rf /var/lib/apt/lists/*

# Asegurar que rustc y cargo estén en el PATH para comandos RUN posteriores y el CMD final
ENV PATH="/root/.cargo/bin:${PATH}"

RUN git config --global --add safe.directory /project

WORKDIR /project

CMD ["/bin/bash"]
