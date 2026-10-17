# Build + run environment for the patched Android. Only a plain Ubuntu base is used.
FROM ubuntu:22.04
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    git-core gnupg flex bison build-essential zip unzip curl ca-certificates rsync bc \
    zlib1g-dev libc6-dev-i386 x11proto-core-dev libx11-dev lib32z1-dev libgl1-mesa-dev \
    libxml2-utils xsltproc fontconfig python3 python3-pip python-is-python3 libncurses5 \
    libssl-dev ccache socat android-tools-adb file lsof procps \
    libpulse0 libnss3 libxcomposite1 libxcursor1 libxi6 libxtst6 libasound2 \
    && rm -rf /var/lib/apt/lists/*

RUN curl -fsSL https://storage.googleapis.com/git-repo-downloads/repo -o /usr/local/bin/repo \
    && chmod +x /usr/local/bin/repo \
    && git config --global user.name asp && git config --global user.email asp@localhost \
    && git config --global color.ui false
RUN curl -LsSf https://astral.sh/uv/install.sh | env UV_INSTALL_DIR=/usr/local/bin sh

COPY . /opt/asp
RUN uv sync --project /opt/asp --no-dev \
    && printf '#!/bin/sh\nexec uv run --project /opt/asp --no-dev asp "$@"\n' > /usr/local/bin/asp \
    && chmod +x /usr/local/bin/asp

ENV USE_CCACHE=1 CCACHE_EXEC=/usr/bin/ccache CCACHE_DIR=/ccache \
    PATH=/aosp/out/host/linux-x86/bin:$PATH
WORKDIR /aosp
CMD ["bash"]
