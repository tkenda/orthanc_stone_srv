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

source /opt/emsdk/emsdk_env.sh

# Use a folder that is writeable by non-root users for the Emscripten cache
export EM_CACHE=/tmp/emscripten-cache

# Make a copy of the read-only folder containing the source code into
# a writeable folder, because of "DownloadPackage.cmake" that writes
# to the "ThirdPartyDownloads" folder next to the "CMakeLists.txt"
cd /source
hg clone /source /tmp/source-writeable

cd /tmp/source-writeable
hg up -c ${STONE_BRANCH}

mkdir /tmp/build
cd /tmp/build

cmake /tmp/source-writeable/Applications/StoneWebViewer/WebAssembly \
      -DCMAKE_BUILD_TYPE=$1 \
      -DORTHANC_STONE_INSTALL_PREFIX=/target/StoneWebViewer \
      -DCMAKE_TOOLCHAIN_FILE=${EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake \
      -DSTATIC_BUILD=ON \
      -G Ninja

ninja -j2 install
