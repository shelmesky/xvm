#!/bin/bash

script=$1

./xsc $1.XSS -N
echo "=============================================="
echo
./xasm $1.XASM
echo "=============================================="
echo
./xvm-console $1.XSE
