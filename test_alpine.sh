#!/bin/sh

set -xe

echo "Running on $(node -v)"

apk add make g++ python
npm test --unsafe-perm
