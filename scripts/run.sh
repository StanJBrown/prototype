set -e

export CC=/usr/bin/clang
export CXX=/usr/bin/clang++

# rm -rf build
mkdir -p build
cd build || return
cmake ..
time make -j8

# cd tests
# ./core-config_test

# sudo make install
