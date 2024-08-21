ARG base_img=ubuntu
ARG base_tag=22.04
FROM --platform=linux/amd64 ${base_img}:${base_tag} AS builder-install
ENV DEBIAN_FRONTEND=noninteractive

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Install 'basic packages'
RUN apt-get update && apt-get -y upgrade
RUN apt-get install -y --no-install-recommends \
    apt-utils \
    locales \
    build-essential \
    cmake \
    gdb \
    libgtest-dev \
    libspdlog-dev \
    curl \
    wget \
    && rm -rf /var/lib/apt/lists/*

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Build GTest library
RUN cd /usr/src/googletest && \
    cmake . && \
    cmake --build . --target install

## # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Install specific version of clang tools (clang-format, clang-tidy)
ARG base_tag=bullseye
ARG llvm_version=14
RUN apt-get update --fix-missing && apt-get -y upgrade
RUN apt-get install -y --no-install-recommends \
    gnupg2 \
    gnupg-agent \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

RUN curl \
    --fail \
    --silent \
    --show-error \
    --location https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN echo "deb http://apt.llvm.org/$base_tag/ llvm-toolchain-$base_tag-$llvm_version main" >> /etc/apt/sources.list.d/llvm.list

RUN apt-get update --fix-missing && apt-get -y upgrade
RUN apt-get install -y --no-install-recommends \
    clang-format-${llvm_version} \
    clang-tidy-${llvm_version} \
    && rm -rf /var/lib/apt/lists/*

RUN ln -s /usr/bin/clang-format-${llvm_version} /usr/local/bin/clang-format
RUN ln -s /usr/bin/clang-tidy-${llvm_version} /usr/local/bin/clang-tidy

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Boost
RUN cd ${HOME} && \
    wget --no-check-certificate --quiet \
        https://boostorg.jfrog.io/artifactory/main/release/1.77.0/source/boost_1_77_0.tar.gz && \
        tar xzf ./boost_1_77_0.tar.gz && \
        cd ./boost_1_77_0 && \
        ./bootstrap.sh && \
        ./b2 install && \
        cd .. && \
        rm -rf ./boost_1_77_0

# # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # # 
# Set up environment
# Language and encoding definition
ENV LANG='en_US.UTF-8' LANGUAGE='en_US:en' LC_ALL='en_US.UTF-8'
RUN echo 'en_US.UTF-8 UTF-8' > /etc/locale.gen && /usr/sbin/locale-gen

# bashrc aliases
RUN echo "alias ll='ls -laGFh'" >> /root/.bashrc

# A Docker volume is used to persist data outside the container's filesystem, 
# allowing data to be shared between containers or preserved even if 
# the container is removed.
# Any data written to this directory during the container's runtime will be 
# stored outside the container, and the data will remain even if the container 
# is stopped or removed.
VOLUME ["/builder/mnt"]

# The WORKDIR instruction is used to specify the default directory where commands 
# executed within the container will run.
# In this case, any subsequent commands executed in the Dockerfile or within the 
# container itself will run in the context of the /builder/mnt directory. 
WORKDIR /builder/mnt

# Final steps and cleanup
RUN apt remove -y \
    wget