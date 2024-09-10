FROM ubuntu:22.04 as build

RUN apt-get -y update && apt-get install -y apt-utils && \
    apt-get install -y -qq -o=Dpkg::Use-Pty=0 build-essential gfortran zlib1g-dev \
    libhdf5-dev libzip-dev ninja-build libcurl4-openssl-dev libboost-all-dev cmake wget python3

COPY . /vcellroot

RUN mkdir -p /vcellroot/build/bin
WORKDIR /vcellroot/build

RUN cmake \
    -G Ninja \
    -DOPTION_TARGET_PYTHON_BINDING=OFF \
    -DOPTION_TARGET_MESSAGING=ON \
    -DOPTION_TARGET_SMOLDYN_SOLVER=OFF \
    -DOPTION_TARGET_FV_SOLVER=ON \
    -DOPTION_TARGET_DOCS=OFF \
    .. && \
    ninja

#RUN ctest
