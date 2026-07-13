FROM archlinux:latest

# Actualizar e instalar dependencias
RUN pacman -Syu --noconfirm && \
    pacman -S --noconfirm \
        base-devel \
        sudo \
        git \
        cmake \
        ninja \
        clang \
        qt6-base \
        qt6-declarative \
        qt6-webengine \
        qt6-svg \
        qt6-tools \
        qt6-5compat \
        curl \
        openssl \
        alsa-lib \
        libpulse \
        jack2 \
        pipewire \
        mesa \
        vulkan-icd-loader \
        libx11 \
        libxi \
        libxext \
        libxfixes \
        libxcursor \
        libxrandr \
        libxss \
        libxtst \
        libdrm \
        libusb \
        dbus \
        bluez \
        libxkbcommon \
        libpng \
        libzip \
        cups \
        wayland \
        libunwind \
        libdecor \
        hicolor-icon-theme \
        wget \
        file \
        python \
        zsync \
        systemd-libs

# Crear usuario no-root para makepkg
RUN useradd -m builder && \
    echo "builder ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

USER builder
WORKDIR /home/builder/build
CMD ["/bin/bash"]
