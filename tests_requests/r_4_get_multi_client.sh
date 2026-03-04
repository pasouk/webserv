#!/bin/bash
for i in {1..100}; do
  #nc localhost 8080 < r_4_get &
  curl "http://localhost:8080/directory/nop" &
  #sleep 0.05
done