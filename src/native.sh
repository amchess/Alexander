#!/bin/bash
make clean
make profile-build ARCH=native COMP=gcc ENV_CXXFLAGS="-DNNUE_EMBEDDING_OFF" -j$(nproc)
strip alexander
mv 'alexander' 'Alexander8.2-native'
make clean