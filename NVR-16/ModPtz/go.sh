#!/bin/sh

cd src/; make clean; make; cd -; 
#cd test/; make clean; make; cp test /mnt/nfs/ -f; cd -
cp obj/libptz.so /mnt/nfs/ -rf;
cp obj/libptz.so ../common/lib/debug -rf;
echo "OK"

