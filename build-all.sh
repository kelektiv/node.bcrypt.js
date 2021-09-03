#!/bin/bash -ue

CLEAN=${CLEAN:-""}
RUN_TESTS=${RUN_TESTS:-false}  # tests presently fail on arm :(

if [ -n "$CLEAN" ]; then
  rm -rf build build-tmp*
  rm -rf prebuilds
fi

npm i
npm i -g prebuildify node-gyp
npm run build

# build for linux/x64:
if [ ! -d prebuilds/linux-x64 ]; then
  docker build -t voltserver/bcryptjs-linux-x64-builder .
  CONTAINER=$(docker create voltserver/bcryptjs-linux-x64-builder)
  docker cp "$CONTAINER:/usr/local/opt/bcrypt-js/prebuilds" .
  docker rm "$CONTAINER"
fi

# build for linux/arm32:
if [ ! -d prebuilds/linux-arm ]; then
  docker build -t voltserver/bcryptjs-linux-arm-builder -f Dockerfile-arm --build-arg RUN_TESTS="$RUN_TESTS" .
  CONTAINER=$(docker create --platform linux/arm/v7 voltserver/bcryptjs-linux-arm-builder)
  docker cp "$CONTAINER:/usr/local/opt/bcrypt-js/prebuilds" .
  docker rm "$CONTAINER"
fi

# build for linux/arm64:
if [ ! -d prebuilds/linux-arm64 ]; then
  docker build -t voltserver/bcryptjs-linux-arm64-builder -f Dockerfile-arm64 --build-arg RUN_TESTS="$RUN_TESTS" .
  CONTAINER=$(docker create --platform linux/arm64/v8 voltserver/bcryptjs-linux-arm64-builder)
  docker cp "$CONTAINER:/usr/local/opt/bcrypt-js/prebuilds" .
  docker rm "$CONTAINER"
fi

ls -lF prebuilds/
