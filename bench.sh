#!/bin/sh

pkill strans
pkill strans-xim
sleep 1

./strans map font &
sleep 1

# warm up glyph cache
./bench/bench bench/bench.keys 100
echo "cache warmed up"

STRANS_PID=$(pgrep -x strans)
perf record -g -o bench/perf.data -p "$STRANS_PID" &
PERF_PID=$!
sleep 1

./bench/bench bench/bench.keys 100

kill -INT $PERF_PID
wait $PERF_PID 2>/dev/null
pkill strans
