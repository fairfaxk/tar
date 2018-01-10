#!/bin/sh

if [ $# -ne 2 ]; then
  echo "usage: dircomp.sh dir1 dir2"
  exit 1
fi

rsync -n -lrtpv --delete $1/ $2/ | head -n -3 | tail -n +2
