#!/bin/sh

cd src/; make clean; make; cd -; 
#cd test/; make clean; make; cp test /mnt/nfs -f; cd -
cp obj/*.so /mnt/nfs -rf;
cp obj/*.so ../Common/lib/debug -rf;
echo "OK"

