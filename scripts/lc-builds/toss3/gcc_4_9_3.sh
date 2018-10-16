#!/bin/bash

##############################################################################
## Copyright (c) 2018, Lawrence Livermore National Security, LLC.
##
## Produced at the Lawrence Livermore National Laboratory
##
## LLNL-CODE-758885
##
## All rights reserved.
##
## This file is part of Comb.
##
## For details, see https://github.com/LLNL/Comb
## Please also see the LICENSE file for MIT license.
##############################################################################

BUILD_SUFFIX=lc_toss3_gcc_4_9_3

rm -rf build_${BUILD_SUFFIX} 2>/dev/null
mkdir build_${BUILD_SUFFIX} && cd build_${BUILD_SUFFIX}

module load cmake/3.9.2 gcc/4.9.3

cmake \
  -DCMAKE_BUILD_TYPE=Release \
  -C ../host-configs/lc-builds/toss3/gcc_4_9_3.cmake \
  -DENABLE_OPENMP=ON \
  -DENABLE_CUDA=OFF \
  -DCMAKE_INSTALL_PREFIX=../install_${BUILD_SUFFIX} \
  "$@" \
  ..