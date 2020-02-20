#!/bin/bash

# Load Submodules (vcpkg, libunwindstack)
git submodule update --init --recursive

if [ $? -ne 0 ]; then
  echo "Orbit: Could not update/initialize all the submodules. Exiting..."
  exit 1
fi

# Build vcpkg
cd external/vcpkg

# Copy the prebuild binaries from user disk
cp -a /vcpkg/scripts scripts
cp -a /vcpkg/downloads downloads
cp -a /vcpkg/installed installed
cp /vcpkg/vcpkg vcpkg
mkdir buildtrees
cp -a /vcpkg/buildtrees/freetype-gl buildtrees/freetype-gl
cd ../..

# Build
./build-release.sh

