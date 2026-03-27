#!/bin/bash

cd build

cmake -DCMAKE_BUILD_TYPE=$0 ..
cmake --build . -- -j8
