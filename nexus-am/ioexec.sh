#!/bin/bash

# T='apps/hello'
# T='tests/timetest'
# T='tests/keytest'
# T='tests/videotest'
# T='apps/slider'
# T='apps/typing'
T='apps/litenes'
cd $AM_HOME/$T
make run -j8
cd -

