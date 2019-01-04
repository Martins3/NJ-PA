#!/bin/bash

BENCH=microbench
cd $AM_HOME/apps/$BENCH
make INPUT=TEST -j8 ARCH=x86-nemu run

