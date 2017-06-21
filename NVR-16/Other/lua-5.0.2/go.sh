#!/bin/sh

echo "start..."
make clean ; make 
cp lib/*.a ../../Common/lib -rf ;
make clean ;
echo "OK"
