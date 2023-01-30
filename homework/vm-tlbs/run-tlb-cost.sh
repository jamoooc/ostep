#!/bin/bash

NUMPAGES=1;

# for each page range from 1..4096
while (( $NUMPAGES <= 8192 )); do
  echo 'Page:' $NUMPAGES;

  # run TLB costs n times and log the output
  for i in {1..100}; do
    ./tlb $NUMPAGES 0 0 > logs/test${i};
  done;

  # sum the test iterations and record the average
  for i in $( seq 1 $NUMPAGES ); do
    sum=0
    # for total n page access
    for file in ./logs/*; do
      sum=$(( $sum + $(sed "${i}q;d" $file) ))
    done;
    echo $i $(( $sum / 100 )) >> sum${NUMPAGES}.txt
  done;

  NUMPAGES=$(( $NUMPAGES * 2 ));
done;

