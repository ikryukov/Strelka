#!/bin/bash
rm -f myopengl
mkdir build
cd build/
cmake ..
cmake --build .
cp myopengl ../myopengl
cd ..
rm -rf build/
