#!/bin/bash
# you will need to `source ...` or `. ./scripts/run-me.sh` this file
echo "Setting up for cd-threading development..."

git fetch && git pull
# NOTE change the target if the makefile target changes
alias valrun="make clean && make && valgrind --leak-check=yes bin/fake"
