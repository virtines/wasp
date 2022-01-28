#!/bin/bash


DATE=$(date '+%Y-%m-%d')

OUTDIR="data/$USERNAME/$HOSTNAME/$DATE"
mkdir -p $OUTDIR
benchmarks=$(ls build/bench/bench* | xargs -n 1 basename | sed 's/bench_//')


for bench in $benchmarks
do
    echo "running $bench"
    build/bench/bench_$bench > $OUTDIR/$bench.csv 
done
