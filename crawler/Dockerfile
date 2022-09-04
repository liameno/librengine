FROM gcc:12.2
FROM ubuntu:20.04

ARG SITES
ARG THREADS=1
ARG CONFIG

ENV DEBIAN_FRONTEND noninteractive
ENV envSITES=$SITES
ENV envTHREADS=$THREADS
ENV envCONFIG=$CONFIG

RUN apt-get update && apt-get -y --no-install-recommends install \
    build-essential gcc cmake libcurl4-openssl-dev libssl-dev git ca-certificates

RUN git clone https://github.com/lexbor/lexbor && \
    cd lexbor && cmake . -DCMAKE_INSTALL_PREFIX=/usr && make && make install

COPY lib /usr/src/librengine/lib
COPY crawler /usr/src/librengine/crawler
WORKDIR /usr/src/librengine/crawler

RUN rm -rf build && mkdir -p build && cd build && cmake .. && make

WORKDIR /usr/src/librengine/website/build
CMD ["sh", "-c", "/usr/src/librengine/crawler/build/crawler \"${envSITES}\" ${envTHREADS} \"${envCONFIG}\""]