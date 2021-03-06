#!/bin/bash
set -e  # halt on first error
BASEDIR=$(dirname "$0")
source "$BASEDIR/config.bash"

REPO_URL=https://github.com/chutsu/apriltags
INC_DEST=${PREFIX}/include/AprilTags
LIB_DEST=${PREFIX}/lib/libapriltags.so
DOWNLOAD_PATH=${PREFIX}/src

install_dependencies()
{
  # Install dependencies
  apt-get install -qq -y \
      cmake \
      libeigen3-dev \
      libv4l-dev
}

install_apriltags()
{
  # Download and build mit apriltags
  cd "$DOWNLOAD_PATH"
  if [ ! -d apriltags ]; then
      git clone $REPO_URL
  fi
  cd apriltags

  # Make
  mkdir -p build
  cd build
  cmake .. \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    $CMAKE_EXTRA_ARGS
  make
  make install
}

# RUN
install_dependencies
install_apriltags
