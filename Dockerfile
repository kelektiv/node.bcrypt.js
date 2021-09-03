#!/bin/echo docker build . -f
# -*- coding: utf-8 -*-
# SPDX-License-Identifier: ISC
# Copyright 2021 VoltServer Inc
#{
# ISC License
# Copyright (c) 2004-2010 by Internet Systems Consortium, Inc. ("ISC")
# Copyright (c) 1995-2003 by Internet Software Consortium
# Permission to use, copy, modify, and /or distribute this software
# for any purpose with or without fee is hereby granted,
# provided that the above copyright notice
#  and this permission notice appear in all copies.
# THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
# WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS.
# IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT,
# OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
# WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS,
# WHETHER IN AN ACTION OF CONTRACT,
# NEGLIGENCE OR OTHER TORTIOUS ACTION,
# ARISING OUT OF OR IN CONNECTION WITH THE USE
# OR PERFORMANCE OF THIS SOFTWARE.
#}
#
# Usage:
#
#   docker build -t voltserver/bcryptjs-linux-x64-builder .
#   docker create --name donut voltserver/bcryptjs-linux-x64-builder
#   # Then copy the artifact to your host:
#   docker cp donut:/usr/local/opt/bcrypt-js/prebuilds .

FROM node:14-bullseye

ENV project bcrypt-js
ENV DEBIAN_FRONTEND noninteractive
ENV LC_ALL en_US.UTF-8
ENV LANG ${LC_ALL}

RUN echo "#log: ${project}: Setup system" \
  && set -x \
  && apt-get update -y \
  && apt-get install -y \
    build-essential \
    python3 \
  && apt-get clean \
  && update-alternatives --install /usr/local/bin/python python /usr/bin/python3 20 \
  && npm i -g prebuildify node-gyp \
  && sync

ADD . /usr/local/opt/${project}
WORKDIR /usr/local/opt/${project}

RUN echo "#log: ${project}: Running build" \
  && set -x \
  && npm i \
  && npm run build

ARG RUN_TESTS=true

RUN if "${RUN_TESTS}"; then \
    echo "#log ${project}: Running tests" \
    && npm test; \
  else \
    echo "#log ${project}: Tests were skipped!"; \
  fi

CMD /bin/bash -l
