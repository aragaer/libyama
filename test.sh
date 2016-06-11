#!/bin/bash -ex

export LD_LIBRARY_PATH=.

test_file=`mktemp --suffix=.yama testXXX`
rm $test_file
result=`./yamaclient read $test_file || :`
test -z "$result"

result=`./yamaclient write $test_file "Hello, world"`
test -z "$result"

result=`./yamaclient read $test_file`
test "$result" = "Hello, world"
rm $test_file
