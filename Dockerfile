FROM ubuntu:22.04

RUN apt-get -y update && apt-get install -y apt-utils && \
    apt-get install -y -qq -o=Dpkg::Use-Pty=0 build-essential gfortran git \
    libhdf5-dev libzip-dev ninja-build libcurl4-openssl-dev libboost-all-dev libbz2-dev cmake python3 valgrind

# Install an alternative Fortran compiler
RUN apt-get install -y gfortran-10 python3 python3-pip

# Get H5PY for tests
RUN pip install h5py

# Set the alternative Fortran compiler as the default
RUN update-alternatives --install /usr/bin/gfortran gfortran /usr/bin/gfortran-10 10

# Fix broken dependencies and reconfigure ca-certificates
RUN apt-get -y update && apt-get install -f && dpkg --configure -a && apt-get clean

COPY . /vcellroot

RUN mkdir -p /vcellroot/build/bin
WORKDIR /vcellroot/build

RUN cmake \
    -G Ninja \
    -DCMAKE_BUILD_TYPE=Debug \
    -DOPTION_TARGET_PYTHON_BINDING=OFF \
    -DOPTION_TARGET_MESSAGING=ON \
    -DOPTION_TARGET_SMOLDYN_SOLVER=ON \
    -DOPTION_TARGET_FV_SOLVER=ON \
    -DOPTION_TARGET_DOCS=OFF \
    -DOPTION_TARGET_TESTS=ON \
    .. && \
    ninja

RUN ctest -VV

WORKDIR /vcellroot/build/bin
ENV PATH="/vcellroot/build/bin:${PATH}"
