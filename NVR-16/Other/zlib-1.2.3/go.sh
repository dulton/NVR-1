#!/bin/sh

echo "start..."
make clean ; make 
cp libz.a ../../Common/lib -rf ;
make clean ;
echo "OK"
