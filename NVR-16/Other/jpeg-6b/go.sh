#!/bin/sh

echo "start..."
#./configure --host=arm-hisiv100-linux --prefix=/home/jpeg/jpeg   --enable-shared  --enable-static
make -f makefile.unix clean ; make -f makefile.unix
cp libjpeg.a ../../Common/lib -rf ;
make -f makefile.unix clean ;
echo "OK"
