#!/bin/bash

echo "starting to sleep"

for x in $(seq 10); do
    echo tick $x
    sleep 1
done

echo "sleeing over!"
echo "this went to stderr" >&2
exit 23
