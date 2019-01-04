#!/bin/bash

# check if we push all the value into the line

# TEST=add-longlong
TEST=hello-str
# TEST=bubble-sort
# div
# TEST=add
# TEST=fact
# TEST=fib
# TEST=goldbach
# TEST=if-else
# TEST=leap-year
# TEST=load-store
# TEST=matrix-mul
# TEST=max
# TEST=min3
# TEST=mov-c # Good
# TEST=movsx # Good
# TEST=mul-longlong
# TEST=pascal
# TEST=prime
# TEST=quick-sort
# TEST=string
# TEST=select-sort
# TEST='shift'
# TEST=shuixianhua
# TEST=sub-longlong
# TEST=sum
# TEST=switch
# TEST=to-lower-case
# TEST=unalign
# TEST=wanshu
# TEST=bit


 
cd $AM_HOME/tests/cputest/
make -j8 ARCH=x86-nemu ALL=$TEST run
cd -


# The Intel-syntax conversion instructions
    # `cbw' -- sign-extend byte in `%al' to word in `%ax',
    # `cwde' -- sign-extend word in `%ax' to long in `%eax',
    # `cwd' -- sign-extend word in `%ax' to long in `%dx:%ax',
    # `cdq' -- sign-extend dword in `%eax' to quad in `%edx:%eax',
# are called `cbtw', `cwtl', `cwtd', and `cltd' in AT&T naming

# nemu/include/common.h
