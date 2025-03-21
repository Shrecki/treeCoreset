#!/bin/bash
git submodule init
git submodule update
cd pybind11
mkdir build && cd build
cmake .. -DCMAKE_INSTALL_PREFIX=../../lib && make -j 4 && make -j 4 install
cd ../../
mkdir build && cd build && cmake .. && make -j 4
