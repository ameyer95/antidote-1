# Multistage building:
# The first compiles the code and fetches the datasets,
# while the second just copies the built objects
# (thus we avoid distributing a large image that includes g++ etc)
#
# While this keeps the distributable image small,
# the user will have to install g++/make to tweak code and rebuild, etc.

# The first image:
# 1) installs g++, make, python3, and wget
# 2) compiles the source code (and tester, which then runs)
# 3) fetches the datasets and preprocesses them
FROM ubuntu:18.04 AS builder

RUN apt-get update \
    && apt-get install -y --no-install-recommends g++ make python3 wget
# No "need" for clean-up since we abandon this base image

COPY . /antidote/
WORKDIR /antidote
RUN make && make test
RUN data/fetch-mnist.sh && data/fetch-uci.sh

# The second (final) image:
# We actually must include python3 since the batch experimentation pipeline uses python scripts
# (ubuntu + apt-get python3 seems to be ~100MB smaller than python:3.8-slim-buster).
FROM ubuntu:18.04

RUN apt-get update \
    && apt-get install -y --no-install-recommends python3 \
    && rm -rf /var/lib/apt/lists*

COPY --from=builder /antidote /antidote
WORKDIR /antidote
