#!/bin/bash

# Stone of Orthanc
# Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
# Department, University Hospital of Liege, Belgium
# Copyright (C) 2017-2021 Osimis S.A., Belgium
#
# This program is free software: you can redistribute it and/or
# modify it under the terms of the GNU Affero General Public License
# as published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Affero General Public License for more details.
#
# You should have received a copy of the GNU Affero General Public License
# along with this program. If not, see <http://www.gnu.org/licenses/>.


set -ex

IMAGE=jodogne/wasm-builder:1.39.17-upstream

if [ "$1" != "Debug" -a "$1" != "Release" ]; then
    echo "Please provide build type: Debug or Release"
    exit -1
fi

if [ -t 1 ]; then
    # TTY is available => use interactive mode
    DOCKER_FLAGS='-i'
fi

ROOT_DIR=`dirname $(readlink -f $0)`/../../..

mkdir -p ${ROOT_DIR}/wasm-binaries

docker run -t ${DOCKER_FLAGS} --rm \
    --user $(id -u):$(id -g) \
    -v ${ROOT_DIR}:/source:ro \
    -v ${ROOT_DIR}/wasm-binaries:/target:rw ${IMAGE} \
    bash /source/Applications/Samples/WebAssembly/docker-internal.sh $1

ls -lR ${ROOT_DIR}/wasm-binaries/
