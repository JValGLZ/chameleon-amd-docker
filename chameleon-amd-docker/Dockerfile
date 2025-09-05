# Use Ubuntu 22.04 as base image
FROM ubuntu:22.04

# Set build arguments
ARG UBUNTU_VERSION=22.04
ARG PAPI_VERSION=7.2.0

# Set environment variables to avoid interactive prompts
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Update package list first
RUN apt-get update

# Install Python setuptools and wheel
RUN apt-get install -y python3-setuptools python3-wheel

# Install environment modules
RUN apt-get install -y environment-modules

# Install wget and curl for downloading
RUN apt-get install -y wget curl

# Install AMD GPU drivers and ROCm
RUN wget https://repo.radeon.com/amdgpu-install/6.4.3/ubuntu/jammy/amdgpu-install_6.4.60403-1_all.deb && \
    apt-get install -y ./amdgpu-install_6.4.60403-1_all.deb && \
    apt-get update && \
    amdgpu-install -y --usecase=rocm --no-dkms && \
    rm -f amdgpu-install_6.4.60403-1_all.deb

# Set ROCm environment variables
ENV ROCM_PATH=/opt/rocm
ENV PATH=$ROCM_PATH/bin:$PATH
ENV LD_LIBRARY_PATH=$ROCM_PATH/lib:$LD_LIBRARY_PATH

# Set PAPI ROCm environment variables
ENV PAPI_ROCM_ROOT=/opt/rocm
ENV PAPI_ROCMSMI_ROOT=/opt/rocm
ENV HSA_TOOLS_LIB=/opt/rocm/lib/librocprofiler64.so

# Install additional dependencies for PAPI with ROCm
RUN apt-get install -y \
    build-essential \
    wget \
    tar \
    gzip \
    gcc \
    g++ \
    make \
    libpfm4-dev \
    linux-tools-generic \
    pkg-config \
    cmake \
    git \
    autoconf \
    automake \
    libtool \
    && rm -rf /var/lib/apt/lists/*

# Download and build PAPI with ROCm support
RUN wget http://icl.utk.edu/projects/papi/downloads/papi-${PAPI_VERSION}.tar.gz && \
    tar -xzf papi-${PAPI_VERSION}.tar.gz && \
    cd papi-${PAPI_VERSION}/src && \
    ./configure --prefix=/usr/local \
                --with-components="rocm rocm_smi" \
                --with-rocm=/opt/rocm \
                --with-rocm-include=/opt/rocm/include \
                --with-rocm-lib=/opt/rocm/lib \
                CPPFLAGS="-I/opt/rocm/include -I/opt/rocm/include/hsa -I/opt/rocm/include/rocprofiler" \
                LDFLAGS="-L/opt/rocm/lib" && \
    make -j$(nproc) && \
    make install && \
    ldconfig && \
    cd / && \
    rm -rf papi-${PAPI_VERSION} papi-${PAPI_VERSION}.tar.gz

# Set library path
ENV LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH

# Create a working directory
WORKDIR /workspace

# Default command
CMD ["/bin/bash"]
