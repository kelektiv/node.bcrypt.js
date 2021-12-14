#!/bin/bash -ue

CLEAN=${CLEAN:-""}
RUN_TESTS=${RUN_TESTS:-true}

if [ -n "$CLEAN" ]; then
  rm -rf build build-tmp*
  rm -rf prebuilds
fi

npm i
npm i -g prebuildify node-gyp
npm run build

# build for linux/x64:
if [ ! -d prebuilds/linux-x64 ]; then
  docker build -t bcryptjs-linux-x64-builder -f Dockerfile-x64 --build-arg RUN_TESTS="$RUN_TESTS" .
  CONTAINER=$(docker create bcryptjs-linux-x64-builder)
  docker cp "$CONTAINER:/usr/local/opt/bcrypt-js/prebuilds" .
  docker rm "$CONTAINER"
fi

# build for linux/arm32:
if [ ! -d prebuilds/linux-arm ]; then
  docker build -t bcryptjs-linux-arm-builder -f Dockerfile-arm --build-arg RUN_TESTS="$RUN_TESTS" .
  CONTAINER=$(docker create --platform linux/arm/v7 bcryptjs-linux-arm-builder)
  docker cp "$CONTAINER:/usr/local/opt/bcrypt-js/prebuilds" .
  docker rm "$CONTAINER"
fi

# build for linux/arm64:
if [ ! -d prebuilds/linux-arm64 ]; then
  docker build -t bcryptjs-linux-arm64-builder -f Dockerfile-arm64 --build-arg RUN_TESTS="$RUN_TESTS" .
  CONTAINER=$(docker create --platform linux/arm64/v8 bcryptjs-linux-arm64-builder)
  docker cp "$CONTAINER:/usr/local/opt/bcrypt-js/prebuilds" .
  docker rm "$CONTAINER"
fi

ls -lF prebuilds/
