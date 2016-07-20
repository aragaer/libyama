#!/bin/bash -ex

export LD_LIBRARY_PATH=.

minute_stamp() {
    set +e
    date -d "`date +'%Y-%m-%d %H:%M'`" +%s
    set -e
}
stamp1=`minute_stamp`

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

stamp2=`minute_stamp`

# Format should be - id date text
result=`./yamaclient list $test_file`

test `wc -l <<<"$result"` == 2
test `grep -Pc '^[0-9a-z]{8}\t\d{4}-\d\d-\d\d \d\d:\d\d\t' <<<"$result"` == 2
cat <<<"$result" | while read id date time text ; do
    stamp=`date -ud "$date $time" +%s`
    test $stamp -ge $stamp1 -a $stamp -le $stamp2
    echo $text
done | diff -u - <(echo -e "Second item\nHello, world")
rm $test_file
