FROM debian:testing

# Aceptar instalar desde testing sin advertencias
RUN echo 'APT::Get::Assume-Yes "true";' > /etc/apt/apt.conf.d/90assumeyes

RUN apt-get update && \
    apt-get install -y \
    build-essential \
    git \
    curl \
    cmake \
    clang \
    qt6-base-dev \
    qt6-tools-dev \
    qt6-declarative-dev \
    qt6-base-dev-tools \
    qtcreator \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /project
CMD ["/bin/bash"]
