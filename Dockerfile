# -*- mode: dockerfile -*-
# Copyright (c) Facebook, Inc. and its affiliates.

FROM ubuntu:20.04

ARG PYTHON_VERSION=3.9
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -yq \
        curl

WORKDIR /opt/conda_setup

RUN curl -o miniconda.sh https://repo.anaconda.com/miniconda/Miniconda3-latest-Linux-x86_64.sh && \
     chmod +x miniconda.sh && \
     ./miniconda.sh -b -p /opt/conda && \
     /opt/conda/bin/conda install -y python=$PYTHON_VERSION && \
     /opt/conda/bin/conda clean -ya

ENV PATH /opt/conda/bin:$PATH

RUN apt-get install -yq \
        build-essential \
        cmake \
        git \
        ninja-build

RUN . /opt/conda/etc/profile.d/conda.sh

COPY . /opt/gilc/

WORKDIR /opt/gilc

RUN rm -rf build

RUN python setup.py install

CMD ["/bin/sh", "-c", "python -u main.py | tee data.dat"]

# Docker commands:
#   docker rm gilc -v
#   docker build -t gilc .
#   docker run --rm --name gilc gilc | tee data.dat
# or
#   docker run -it -v `pwd`:/opt/src --entrypoint /bin/bash gilc
