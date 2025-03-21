#!/bin/bash
git submodule init
git submodule update
cd pybind11
mkdir build && cd build
cmake .. && make -j 4 && make -DCMAKE_INSTALL_PREFIX=../../lib install
cd ../../
mkdir build && cd build && cmake .. && make -j 4
