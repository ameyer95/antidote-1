FROM ubuntu:18.04

RUN apt-get update \
    && apt-get install -y --no-install-recommends g++ make python3 wget \
    && rm -rf /var/apt/lists*

COPY . /antidote/

WORKDIR /antidote
RUN make && data/fetch-mnist.sh && data/fetch-uci.sh
