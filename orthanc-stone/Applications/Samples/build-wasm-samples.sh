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


#
# usage:
# to build the samples in RelWithDebInfo:
# ./build-wasm-samples.sh
#
# to build the samples in Release:
# ./build-wasm-samples.sh Release
#

set -e

if [ ! -d "WebAssembly" ]; then
  echo "This script must be run from the Samples folder one level below orthanc-stone"
  exit 1
fi


currentDir=$(pwd)
samplesRootDir=$(pwd)
devrootDir=$(pwd)/../../

buildType=${1:-RelWithDebInfo}
buildFolderName="$devrootDir/out/build-stone-wasm-samples-$buildType"
installFolderName="$devrootDir/out/install-stone-wasm-samples-$buildType"

mkdir -p $buildFolderName
# change current folder to the build folder
pushd $buildFolderName

# configure the environment to use Emscripten
source ~/apps/emsdk/emsdk_env.sh

emcmake cmake -G "Ninja" \
  -DCMAKE_BUILD_TYPE=$buildType \
  -DORTHANC_STONE_INSTALL_PREFIX=$installFolderName \
  -DSTATIC_BUILD=ON -DALLOW_DOWNLOADS=ON \
  $samplesRootDir/WebAssembly

# perform build + installation
ninja
ninja install

# restore the original working folder
popd

echo "If all went well, the output files can be found in $installFolderName:"

ls $installFolderName
