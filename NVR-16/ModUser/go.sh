#!/bin/sh

cd src/; make clean; make; cp *.so /mnt/nfs/ -f; cp *.so ../../common/lib/debug -f; cd -; 
cd test/; make clean; make; cp test /mnt/nfs/ -f; cd -
echo "OK"

