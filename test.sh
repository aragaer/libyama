#!/bin/bash -ex

export LD_LIBRARY_PATH=.

test_file=`mktemp --suffix=.yama testXXX`
rm $test_file
result=`./yamaclient read $test_file || :`
test -z "$result"

result=`./yamaclient write $test_file "Hello, world"`
test -z "$result"

# test magic
test `dd if=$test_file bs=4 count=1 2>/dev/null` == "YAMA"

result=`./yamaclient read $test_file`
test "$result" = "Hello, world"

result=`./yamaclient write $test_file "Second item"`
test -z "$result"

diff -u <(./yamaclient read $test_file) - <<EOF
Second item
Hello, world
EOF

rm $test_file
