#!/bin/bash

# Download repository
rm -f -R orthanc-stone
hg clone https://hg.orthanc-server.com/orthanc-stone

# Build WebAssembly front-end
sudo sh ./orthanc-stone/Applications/StoneWebViewer/WebAssembly/docker-build.sh Release
rm -f -R front
cp -R ./orthanc-stone/wasm-binaries/StoneWebViewer ./front